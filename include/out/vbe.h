// vbe.h - VBE (framebuffer) driver for Alphix
#ifndef _ALPHIX_OUT_VBE_H
#define _ALPHIX_OUT_VBE_H

#include <stdint.h>

// Инициализация VBE драйвера
void vbe_init(uint32_t magic, uint64_t mboot_addr);

// Базовые функции рисования
void vbe_put_pixel(uint32_t x, uint32_t y, uint8_t color);
void vbe_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t color);
void vbe_clear_screen(uint8_t color);

// Экспортированные переменные
extern volatile uint8_t* framebuffer;
extern uint32_t fb_width;
extern uint32_t fb_height;
extern uint32_t fb_pitch;
extern uint8_t fb_bpp;
extern const uint32_t vga_palette[16];

#endif // _ALPHIX_OUT_VBE_H 