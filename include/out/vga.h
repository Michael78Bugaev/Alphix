// Alphix kernel
// VGA driver for Alphix
// include/out/vga.h

#ifndef _ALPHIX_VGA_H
#define _ALPHIX_VGA_H

#include "../kernel/types.h"

// VGA цвета (16 цветов)
typedef enum {
    VGA_BLACK = 0,
    VGA_BLUE = 1,
    VGA_GREEN = 2,
    VGA_CYAN = 3,
    VGA_RED = 4,
    VGA_MAGENTA = 5,
    VGA_BROWN = 6,
    VGA_LIGHT_GREY = 7,
    VGA_DARK_GREY = 8,
    VGA_LIGHT_BLUE = 9,
    VGA_LIGHT_GREEN = 10,
    VGA_LIGHT_CYAN = 11,
    VGA_LIGHT_RED = 12,
    VGA_LIGHT_MAGENTA = 13,
    VGA_YELLOW = 14,
    VGA_WHITE = 15
} vga_color_t;

// Структура для описания видеорежима
typedef struct {
    uint32_t width;
    uint32_t height;
    uint8_t bpp;  // Биты на пиксель
    uint8_t* framebuffer;
    uint32_t pitch;
} vga_mode_t;

// Инициализация VGA
void vga_init(uint8_t* framebuffer, uint32_t width, uint32_t height, uint32_t pitch, uint8_t bpp);

// Базовые функции рисования
void vga_put_pixel(uint32_t x, uint32_t y, vga_color_t color);
void vga_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, vga_color_t color);
void vga_clear_screen(vga_color_t color);

// Получение информации о текущем режиме
vga_mode_t* vga_get_mode(void);

#endif // _ALPHIX_VGA_H 