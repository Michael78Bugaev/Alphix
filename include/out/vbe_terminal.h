// vbe_terminal.h - VBE terminal API for Alphix
#ifndef _ALPHIX_OUT_VBE_TERMINAL_H
#define _ALPHIX_OUT_VBE_TERMINAL_H

#include <stdint.h>

// Экранный буфер символов, max 128x96
#define MAX_ROWS 128
#define MAX_COLS 128

// Структура для хранения символа с цветами
typedef struct {
    char ch;
    uint8_t fg;
    uint8_t bg;
} screen_cell_t;

// Инициализация терминала: magic и адрес multiboot, а также цвета (fg, bg)
void vbe_terminal_init(uint32_t magic, uint64_t mboot_addr, uint8_t fg, uint8_t bg);

// Вывод одного символа
void vbe_terminal_putchar(char c);

// Вывод нуль-терминированной строки
void vbe_terminal_puts(const char *s);

// Форматированный вывод
void vbe_printf(const char *fmt, ...);

// Мигающий курсор: toggle и цикл blink
void vbe_terminal_toggle_cursor(void);
void vbe_terminal_blink_cursor(void);

void vbe_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t color);
void vbe_clear_screen(uint8_t color);

// Дополнительные функции терминала
void vbe_terminal_putc_at(uint32_t col, uint32_t row, char c);
void vbe_terminal_putc_at_colors(uint32_t col, uint32_t row, char c, uint8_t fg, uint8_t bg);

// Глобальные координаты курсора и терминала (экспортируются для отладочных модулей)
extern uint32_t term_x, term_y;
extern uint32_t cursor_x, cursor_y;
extern uint32_t term_cols, term_rows;
extern int cursor_on;
extern uint8_t term_fg, term_bg;
extern screen_cell_t screen_buf[MAX_ROWS][MAX_COLS];

#endif // _ALPHIX_OUT_VBE_TERMINAL_H 