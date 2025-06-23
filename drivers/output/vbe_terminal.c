// Alphix kernel
// VBE terminal
// drivers/output/vbe_terminal.c

#include <stdint.h>
#include <out/vbe.h>
#include <out/vbe_terminal.h>
#include <bitmps/termfont8x8.h>
#include <stdarg.h>
#include <debug.h>
#include <stdbool.h>
#include <string.h>

#define FONT_WIDTH  _ALPHIX_BITMAPS_TERMFONT8X8_H_WIDTH
#define FONT_HEIGHT _ALPHIX_BITMAPS_TERMFONT8X8_H_HEIGHT

uint32_t term_cols, term_rows;
uint32_t term_x, term_y;
// Координаты мигающего курсора
uint32_t cursor_x, cursor_y;
uint8_t term_fg, term_bg;

// Экранный буфер символов
screen_cell_t screen_buf[MAX_ROWS][MAX_COLS];

// Мигающий курсор
int cursor_on = 0;
// Сохранённый символ под курсором и его цвета
static char saved_char = ' ';
static uint8_t saved_fg, saved_bg;
static bool cursor_char_saved = false;

// Функция инверсии цвета для VGA палитры
static uint8_t invert_color(uint8_t color) {
    // Простая инверсия для 16-цветной VGA палитры
    return color ^ 0x0F;  // XOR с 15 инвертирует все 4 бита
}

// Вспомогательная отрисовка символа с заданными цветами
void vbe_terminal_putc_at_colors(uint32_t col, uint32_t row, char c, uint8_t fg, uint8_t bg) {
    if (col >= term_cols || row >= term_rows) return;
    uint32_t x0 = col * FONT_WIDTH;
    uint32_t y0 = row * FONT_HEIGHT;
    if (c == ' ') {
        // очищаем фоном
        vbe_fill_rect(x0, y0, FONT_WIDTH, FONT_HEIGHT, bg);
        return;
    }
    uint8_t uc = (uint8_t)c;
    if (uc >= _ALPHIX_BITMAPS_TERMFONT8X8_H_SIZE) return;
    for (uint32_t ry = 0; ry < FONT_HEIGHT; ry++) {
        uint8_t bits = font8x8_basic[uc][ry];
        for (uint32_t cx = 0; cx < FONT_WIDTH; cx++) {
            uint8_t color = (bits & (1 << cx)) ? fg : bg;
            vbe_put_pixel(x0 + cx, y0 + ry, color);
        }
    }
}

// Вспомогательная отрисовка символа в произвольной позиции
void vbe_terminal_putc_at(uint32_t col, uint32_t row, char c) {
    vbe_terminal_putc_at_colors(col, row, c, term_fg, term_bg);
    // Обновляем буфер экрана тоже
    if (col < term_cols && row < term_rows) {
        screen_buf[row][col].ch = c;
        screen_buf[row][col].fg = term_fg;
        screen_buf[row][col].bg = term_bg;
    }
}

// Отрисовка символа и обновление буфера
void vbe_terminal_putchar(char c) {
    // Скрываем курсор перед изменением экрана
    bool was_on = cursor_on;
    if (was_on) vbe_terminal_toggle_cursor();
    // Обработка Backspace
    if (c == '\b') {
        if (term_x > 0) term_x--;
        else if (term_y > 0) { term_y--; term_x = term_cols - 1; }
        // Очистить ячейку
        vbe_fill_rect(term_x * FONT_WIDTH, term_y * FONT_HEIGHT,
                      FONT_WIDTH, FONT_HEIGHT, term_bg);
        screen_buf[term_y][term_x].ch = ' ';
        screen_buf[term_y][term_x].fg = term_fg;
        screen_buf[term_y][term_x].bg = term_bg;
        // Обновить позицию курсора
        cursor_x = term_x;
        cursor_y = term_y;
        // Показать курсор в новой позиции
        if (was_on) vbe_terminal_toggle_cursor();
        return;
    }
    if (c == '\n') {
        term_x = 0;
        term_y++;
        if (term_y == term_rows) {
            term_y = 0;
            vbe_clear_screen(term_bg);
            // очистить буфер
            for (uint32_t r=0; r<term_rows; r++) {
                for (uint32_t cc=0; cc<term_cols; cc++) {
                    screen_buf[r][cc].ch = ' ';
                    screen_buf[r][cc].fg = term_fg;
                    screen_buf[r][cc].bg = term_bg;
                }
            }
        }
        // Обновить позицию курсора
        cursor_x = term_x;
        cursor_y = term_y;
        if (was_on) vbe_terminal_toggle_cursor();
        return;
    }
    if (c == '\r') { term_x = 0; 
        // Обновить позицию курсора
        cursor_x = term_x;
        cursor_y = term_y;
        if (was_on) vbe_terminal_toggle_cursor();
        return;
    }
    if ((uint8_t)c >= _ALPHIX_BITMAPS_TERMFONT8X8_H_SIZE) return;

    // рисуем
    vbe_terminal_putc_at(term_x, term_y, c);
    screen_buf[term_y][term_x].ch = c;
    screen_buf[term_y][term_x].fg = term_fg;
    screen_buf[term_y][term_x].bg = term_bg;

    term_x++;
    if (term_x == term_cols) {
        term_x = 0;
        term_y++;
        if (term_y == term_rows) {
            term_y = 0;
            vbe_clear_screen(term_bg);
            for (uint32_t r=0; r<term_rows; r++) {
                for (uint32_t cc=0; cc<term_cols; cc++) {
                    screen_buf[r][cc].ch = ' ';
                    screen_buf[r][cc].fg = term_fg;
                    screen_buf[r][cc].bg = term_bg;
                }
            }
        }
    }
    // Обновить позицию курсора
    cursor_x = term_x;
    cursor_y = term_y;
    // Показать курсор после печати
    if (was_on) vbe_terminal_toggle_cursor();
}

void vbe_terminal_init(uint32_t magic, uint64_t mboot_addr, uint8_t fg, uint8_t bg) {
    vbe_init(magic, mboot_addr);
    term_cols = fb_width / FONT_WIDTH;
    term_rows = fb_height / FONT_HEIGHT;
    term_x = term_y = 0;
    cursor_x = term_x;
    cursor_y = term_y;
    term_fg = fg;
    term_bg = bg;
    vbe_clear_screen(bg);
    // инициализируем буфер пробелами
    for (uint32_t r=0; r<term_rows; r++) {
        for (uint32_t c=0; c<term_cols; c++) {
            screen_buf[r][c].ch = ' ';
            screen_buf[r][c].fg = fg;
            screen_buf[r][c].bg = bg;
        }
    }
}

// Переключение курсора
void vbe_terminal_toggle_cursor(void) {
    if (!cursor_on) {
        // Сохраняем символ и его реальные цвета под курсором
        saved_char = screen_buf[cursor_y][cursor_x].ch;
        saved_fg = screen_buf[cursor_y][cursor_x].fg;
        saved_bg = screen_buf[cursor_y][cursor_x].bg;
        cursor_char_saved = true;
        
        // show: рисуем символ с инвертированными цветами
        uint8_t inv_fg = invert_color(saved_fg);
        uint8_t inv_bg = invert_color(saved_bg);
        vbe_terminal_putc_at_colors(cursor_x, cursor_y, saved_char, inv_fg, inv_bg);
        cursor_on = 1;
    } else {
        // hide: восстанавливаем оригинальный символ с его цветами
        if (cursor_char_saved) {
            vbe_terminal_putc_at_colors(cursor_x, cursor_y, saved_char, saved_fg, saved_bg);
            cursor_char_saved = false;
        }
        cursor_on = 0;
    }
}

// Вывод строки
void vbe_terminal_puts(const char *s) {
    while (*s) {
        vbe_terminal_putchar(*s++);
    }
}

// Вспомогательная функция: вывод беззнакового числа в заданной системе счисления
static void vbe_put_uint(unsigned int value, int base) {
    char buf[32];
    const char *digits = "0123456789abcdef";
    int i = 0;
    if (value == 0) {
        vbe_terminal_putchar('0');
        return;
    }
    while (value) {
        buf[i++] = digits[value % base];
        value /= base;
    }
    while (i > 0) {
        vbe_terminal_putchar(buf[--i]);
    }
}

// printf-подобная функция вывода
void vbe_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    for (const char *p = fmt; *p; p++) {
        if (*p != '%') {
            vbe_terminal_putchar(*p);
            continue;
        }
        p++;
        if (!*p) break;
        switch (*p) {
        case 'c': {
            char c = (char)va_arg(args, int);
            vbe_terminal_putchar(c);
            break;
        }
        case 's': {
            char *s = va_arg(args, char*);
            vbe_terminal_puts(s);
            break;
        }
        case 'd':
        case 'i': {
            int val = va_arg(args, int);
            if (val < 0) {
                vbe_terminal_putchar('-');
                vbe_put_uint(-val, 10);
            } else {
                vbe_put_uint(val, 10);
            }
            break;
        }
        case 'u': {
            unsigned int val = va_arg(args, unsigned int);
            vbe_put_uint(val, 10);
            break;
        }
        case 'x':
        case 'X': {
            unsigned int val = va_arg(args, unsigned int);
            vbe_put_uint(val, 16);
            break;
        }
        case '%': {
            vbe_terminal_putchar('%');
            break;
        }
        default:
            vbe_terminal_putchar('%');
            vbe_terminal_putchar(*p);
            break;
        }
    }
    va_end(args);
}

// Бесконечный цикл мигания курсора с задержкой
void vbe_terminal_blink_cursor(void) {
    const uint64_t delay = 5000000ULL;
    for (;;) {
        for (volatile uint64_t d = 0; d < delay; d++);
        vbe_terminal_toggle_cursor();
    }
}