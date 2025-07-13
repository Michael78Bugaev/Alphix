// Alphix kernel
// Kernel printf implementation
// cpu/kprintf.c

#include <kprintf.h>
#include <out/vga.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <in/keyboard.h>
#include <string.h>
#include <stdbool.h>
#include <cpu/cpu.h>
#include <cpu/idt.h>
#include <in/keys.h>
#include <mem/heap.h>

static void kputc(char c) {
    kputchar(c, 0x07);
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
static size_t kscan_pos = 0;
static int enter = 0;
static char kscan_buf[512];

void base_init()
{
    //kscan_buf = kmalloc(512);
}

static void backspace(char *buf)
{
    if (buf[0] != '\0')
    {
        buf[strlen(buf) - 1] = '\0';
    }
}

static void kscan_handler(cpu_registers_t *regs)
{
    (void)regs; // подавляем предупреждение о неиспользуемом параметре
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
            kscan_pos--;
        }
        return;
    }
    if (scancode == KEY_RIGHT && pressed) {
        if (kscan_pos < len) {
            kscan_pos++;
        }
        return;
    }
    
    switch (scancode)
    {
        case 0x0E: // Backspace
            if (pressed && kscan_pos > 0) {
                memmove(&kscan_buf[kscan_pos-1], &kscan_buf[kscan_pos], len - kscan_pos + 1);
                kscan_pos--;
                kscan_buf[len-1] = '\0'; // корректное завершение строки
                // Перерисовываем строку
                uint16_t cursor_pos = get_cursor();
                uint16_t start_pos = cursor_pos - (len * 2);
                set_cursor(start_pos);
                for (size_t i = 0; i < len; i++) {
                    kputchar(kscan_buf[i], 0x07);
                }
                kputchar(' ', 0x07); // Очищаем последний символ
                set_cursor(start_pos + (kscan_pos * 2));
            }
            break;
        case 0x1C: // Enter
            if (pressed) { 
                kprintf("\n"); 
                enter = 1; 
                kscan_pos = 0; // сброс позиции
            }
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
                    strins(kscan_buf, c, kscan_pos);
                    kputchar(c, 0x07);
                    kscan_pos++;
                }
            }
            break;
    }
}

static int getch_end = 0;
static char getch_char = 0;

void getch_handler(cpu_registers_t *regs)
{
    (void)regs; // подавляем предупреждение о неиспользуемом параметре
    uint8_t code = inb(0x60);
    bool pressed = !(code & 0x80);
    uint8_t scancode = code & 0x7F;
    
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
    
    // Обрабатываем только нажатия клавиш (не отпускания)
    if (!pressed) {
        return;
    }
    
    // Обработка специальных клавиш
    switch (scancode) {
        case KEY_ENTER:
            getch_char = '\n';
            getch_end = 1;
            return;
        case KEY_BACKSPACE:
            getch_char = '\b';
            getch_end = 1;
            return;
        case KEY_TAB:
            getch_char = '\t';
            getch_end = 1;
            return;
        case KEY_ESC:
            getch_char = 27; // ESC
            getch_end = 1;
            return;
        case KEY_UP:
            getch_char = 72; // Up arrow
            getch_end = 1;
            return;
        case KEY_DOWN:
            getch_char = 80; // Down arrow
            getch_end = 1;
            return;
        case KEY_LEFT:
            getch_char = 75; // Left arrow
            getch_end = 1;
            return;
        case KEY_RIGHT:
            getch_char = 77; // Right arrow
            getch_end = 1;
            return;
        case KEY_F1:
            getch_char = 59; // F1
            getch_end = 1;
            return;
        case KEY_F2:
            getch_char = 60; // F2
            getch_end = 1;
            return;
        case KEY_F3:
            getch_char = 61; // F3
            getch_end = 1;
            return;
        case KEY_F4:
            getch_char = 62; // F4
            getch_end = 1;
            return;
        case KEY_F5:
            getch_char = 63; // F5
            getch_end = 1;
            return;
        case KEY_F6:
            getch_char = 64; // F6
            getch_end = 1;
            return;
        case KEY_F7:
            getch_char = 65; // F7
            getch_end = 1;
            return;
        case KEY_F8:
            getch_char = 66; // F8
            getch_end = 1;
            return;
        case KEY_F9:
            getch_char = 67; // F9
            getch_end = 1;
            return;
        case KEY_F10:
            getch_char = 68; // F10
            getch_end = 1;
            return;
        case KEY_F11:
            getch_char = 69; // F11
            getch_end = 1;
            return;
        case KEY_F12:
            getch_char = 70; // F12
            getch_end = 1;
            return;
        default:
            // Обработка обычных символов
            char c;
            if (shift || capslock) {
                c = get_acsii_high(scancode);
            } else {
                c = get_acsii_low(scancode);
            }
            if (c) {
                getch_char = c;
                getch_end = 1;
            }
            break;
    }
}

int kgetch()
{
    // Сбрасываем состояние
    getch_end = 0;
    getch_char = 0;
    
    // Регистрируем обработчик прерывания клавиатуры
    idt_register_handler(33, getch_handler);
    
    // Ждем нажатия клавиши
    while (!getch_end) {
        // Просто ждем, пока getch_handler не установит getch_end = 1
    }
    
    // Отключаем обработчик
    idt_uninstall_handler(33);
    
    return getch_char;
}

void kscan(char *buf)
{
    enter = 0;
    kprintf("kscan: ");
    kscan_pos = 0;
    strnone(kscan_buf); // Очищаем буфер перед вводом
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
    kprintf("You typed: %s\n", kscan_buf);
    strncpy(buf, kscan_buf, strlen(kscan_buf));
    buf[strlen(kscan_buf)] = '\0'; // Гарантируем завершение строки
}