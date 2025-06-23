// Alphix kernel
// Kernel printf implementation
// cpu/kprintf.c

#include <kprintf.h>
#include <out/vbe_terminal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <in/keyboard.h>
#include <string.h>
#include <stdbool.h>
#include <cpu/cpu.h>
#include <cpu/idt.h>
#include <in/keys.h>

static void kputc(char c) {
    vbe_terminal_putchar(c);
}

void kvprintf(const char *fmt, va_list ap) {
    const char *p = fmt;
    while (*p) {
        if (*p != '%') {
            kputc(*p++);
            continue;
        }
        p++; // skip '%'
        // Flags
        bool left = false;
        bool zero = false;
        if (*p == '-') { left = true; p++; }
        if (*p == '0') { zero = true; p++; }
        // Width
        int width = 0;
        while (*p >= '0' && *p <= '9') {
            width = width * 10 + (*p++ - '0');
        }
        // Precision
        int precision = -1;
        if (*p == '.') {
            p++;
            precision = 0;
            while (*p >= '0' && *p <= '9') {
                precision = precision * 10 + (*p++ - '0');
            }
        }
        // Length modifier - skip 'l'
        if (*p == 'l') {
            p++;
        }
        char spec = *p++;
        switch (spec) {
            case 'd': case 'i': {
                long val = va_arg(ap, long);
                unsigned long u;
                bool neg = false;
                if (val < 0) { neg = true; u = (unsigned long)(-val); }
                else { u = (unsigned long)val; }
                char numbuf[32]; int len = 0;
                if (u == 0) numbuf[len++] = '0';
                while (u) { numbuf[len++] = '0' + (u % 10); u /= 10; }
                if (neg) numbuf[len++] = '-';
                int precpad = (precision > len) ? (precision - len) : 0;
                int total_len = len + precpad;
                int pad = (width > total_len) ? (width - total_len) : 0;
                if (!left) {
                    char padc = zero ? '0' : ' ';
                    for (int i = 0; i < pad; i++) kputc(padc);
                }
                for (int i = 0; i < precpad; i++) kputc('0');
                for (int i = len - 1; i >= 0; i--) kputc(numbuf[i]);
                if (left) {
                    for (int i = 0; i < pad; i++) kputc(' ');
                }
                break;
            }
            case 'u': {
                unsigned long u = va_arg(ap, unsigned long);
                char numbuf[32]; int len = 0;
                if (u == 0) numbuf[len++] = '0';
                while (u) { numbuf[len++] = '0' + (u % 10); u /= 10; }
                int precpad = (precision > len) ? (precision - len) : 0;
                int total_len = len + precpad;
                int pad = (width > total_len) ? (width - total_len) : 0;
                if (!left) {
                    char padc = zero ? '0' : ' ';
                    for (int i = 0; i < pad; i++) kputc(padc);
                }
                for (int i = 0; i < precpad; i++) kputc('0');
                for (int i = len - 1; i >= 0; i--) kputc(numbuf[i]);
                if (left) {
                    for (int i = 0; i < pad; i++) kputc(' ');
                }
                break;
            }
            case 'x': case 'X': {
                unsigned long u = va_arg(ap, unsigned long);
                char numbuf[32]; int len = 0;
                if (u == 0) numbuf[len++] = '0';
                while (u) { int d = u & 0xF; numbuf[len++] = d < 10 ? '0' + d : 'a' + (d - 10); u >>= 4; }
                int precpad = (precision > len) ? (precision - len) : 0;
                int total_len = len + precpad;
                int pad = (width > total_len) ? (width - total_len) : 0;
                if (!left) {
                    char padc = zero ? '0' : ' ';
                    for (int i = 0; i < pad; i++) kputc(padc);
                }
                for (int i = 0; i < precpad; i++) kputc('0');
                for (int i = len - 1; i >= 0; i--) kputc(numbuf[i]);
                if (left) {
                    for (int i = 0; i < pad; i++) kputc(' ');
                }
                break;
            }
            case 'p': {
                void *ptr = va_arg(ap, void*);
                unsigned long u = (unsigned long)ptr;
                kputc('0'); kputc('x');
                for (int i = (int)(sizeof(void*) * 2) - 1; i >= 0; i--) {
                    int shift = i * 4;
                    int d = (u >> shift) & 0xF;
                    kputc(d < 10 ? '0' + d : 'a' + (d - 10));
                }
                break;
            }
            case 's': {
                char *s = va_arg(ap, char*);
                if (!s) s = "(null)";
                int len = 0;
                while (s[len] && (precision < 0 || len < precision)) len++;
                int pad = (width > len) ? (width - len) : 0;
                if (!left) for (int i = 0; i < pad; i++) kputc(' ');
                for (int i = 0; i < len; i++) kputc(s[i]);
                if (left) for (int i = 0; i < pad; i++) kputc(' ');
                break;
            }
            case 'c': {
                char c = (char)va_arg(ap, int);
                int pad = (width > 1) ? (width - 1) : 0;
                if (!left) for (int i = 0; i < pad; i++) kputc(' ');
                kputc(c);
                if (left) for (int i = 0; i < pad; i++) kputc(' ');
                break;
            }
            case '%': {
                kputc('%');
                break;
            }
            default: {
                kputc('%'); kputc(spec);
                break;
            }
        }
    }
}

void kprintf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    kvprintf(fmt, ap);
    va_end(ap);
} 

static int capslock = 0;
static int shift = 0;
static int ctrl = 0;
static int alt = 0;
static int input_start_x, input_start_y;
static size_t edit_pos;
static char kscan_buf[512];
static size_t kscan_pos = 0;
static int enter = 0;

static void backspace(char *buf)
{
    if (buf[0] != '\0')
    {
        buf[strlen(buf) - 1] = '\0';
    }
}

static void redraw_input_line()
{
    // hide cursor if on
    if (cursor_on) vbe_terminal_toggle_cursor();
    // move to start of line (assumes term_x/term_y tracking)
    while (term_x > 0) {
        term_x--; cursor_x--; vbe_terminal_putc_at(term_x, term_y, ' ');
    }
    // rewrite buffer
    for (size_t i=0;i<strlen(kscan_buf);i++) {
        vbe_terminal_putchar(kscan_buf[i]);
    }
    // set term_x,y to after buffer
    // they already updated in putchar
    // adjust cursor position
    cursor_x = kscan_pos;
    if (cursor_on) vbe_terminal_toggle_cursor();
}

static void kscan_handler(cpu_registers_t *regs)
{
    uint8_t code = inb(0x60);
    bool pressed = !(code & 0x80);
    uint8_t scancode = code & 0x7F;
    size_t len = strlen(kscan_buf);
    
    // Обработка модификаторов
    switch (scancode) {
        case KEY_LSHIFT:
        case KEY_RSHIFT:
            shift = pressed ? 1 : 0;
            return;
        case KEY_CAPS_LOCK:
            if (pressed) {
                capslock = !capslock;
            }
            return;
        case KEY_LCTRL:
            ctrl = pressed ? 1 : 0;
            return;
        case KEY_LALT:
            alt = pressed ? 1 : 0;
            return;
    }
    
    // Обработка стрелок влево/вправо
    if (scancode == KEY_LEFT && pressed) {
        if (kscan_pos > 0) {
            if (cursor_on) vbe_terminal_toggle_cursor();
            kscan_pos--;
            cursor_x = input_start_x + kscan_pos;
            cursor_y = input_start_y;
            if (cursor_on) vbe_terminal_toggle_cursor();
        }
        return;
    }
    if (scancode == KEY_RIGHT && pressed) {
        if (kscan_pos < len) {
            if (cursor_on) vbe_terminal_toggle_cursor();
            kscan_pos++;
            cursor_x = input_start_x + kscan_pos;
            cursor_y = input_start_y;
            if (cursor_on) vbe_terminal_toggle_cursor();
        }
        return;
    }
    
    switch (scancode)
    {
        case 0x0E:
            if (pressed && kscan_pos > 0) {
                if (cursor_on) vbe_terminal_toggle_cursor();
                memmove(&kscan_buf[kscan_pos-1], &kscan_buf[kscan_pos], len - kscan_pos + 1);
                kscan_pos--;
                for (size_t i = 0; i < len; i++) {
                    vbe_terminal_putc_at(input_start_x + i, input_start_y, kscan_buf[i]);
                }
                vbe_terminal_putc_at(input_start_x + len - 1, input_start_y, ' ');
                cursor_x = input_start_x + kscan_pos;
                cursor_y = input_start_y;
                if (cursor_on) vbe_terminal_toggle_cursor();
            }
            break;
        case 0x1C:
            if (pressed) { kprintf("\n"); enter = 1; }
            break;
        default:
            if (pressed)
            {
                char c;
                if (shift || capslock)
                {
                    c = get_acsii_high(scancode);
                }
                else
                {
                    c = get_acsii_low(scancode);
                }
                if (c) {
                    // Вставляем символ в позицию курсора
                    if (cursor_on) vbe_terminal_toggle_cursor();
                    strins(kscan_buf, c, kscan_pos);
                    // Перерисовываем строку от позиции курсора
                    for (size_t i = kscan_pos; i <= strlen(kscan_buf); i++) {
                        vbe_terminal_putc_at(input_start_x + i, input_start_y, kscan_buf[i]);
                    }
                    kscan_pos++;
                    cursor_x = input_start_x + kscan_pos;
                    cursor_y = input_start_y;
                    if (cursor_on) vbe_terminal_toggle_cursor();
                }
            }
            break;
    }
}

unsigned char *kscan()
{
    strnone(kscan_buf);
    enter = 0;
    kprintf("kscan: ");
    input_start_x = term_x;
    input_start_y = term_y;
    kscan_pos = 0;
    while (enter != 1)
    {
        idt_register_handler(33, &kscan_handler);  
        if (enter == 1)
        {
            idt_uninstall_handler(33);
            break;
        }
    }
    idt_uninstall_handler(33);
    return kscan_buf;
}