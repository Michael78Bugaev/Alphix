// Alphix kernel
// VBE (framebuffer) driver for Alphix
// drivers/output/vbe.c

#ifndef _ALPHIX_DRIVERS_OUTPUT_VBE_H
#define _ALPHIX_DRIVERS_OUTPUT_VBE_H

#include <multiboot2.h>
#include <stdint.h>
#include <out/vbe.h>

// Глобальные переменные для VBE
volatile uint8_t* framebuffer = NULL;
uint32_t fb_width = 0;
uint32_t fb_height = 0;
uint32_t fb_pitch = 0;
uint8_t fb_bpp = 0;

// Палитра VGA (16 цветов)
const uint32_t vga_palette[16] = {
    0x000000, // Чёрный
    0x0000AA, // Синий
    0x00AA00, // Зелёный
    0x00AAAA, // Голубой
    0xAA0000, // Красный
    0xAA00AA, // Пурпурный
    0xAA5500, // Коричневый
    0xAAAAAA, // Светло-серый
    0x555555, // Тёмно-серый
    0x5555FF, // Ярко-синий
    0x55FF55, // Ярко-зелёный
    0x55FFFF, // Ярко-голубой
    0xFF5555, // Ярко-красный
    0xFF55FF, // Ярко-пурпурный
    0xFFFF55, // Жёлтый
    0xFFFFFF  // Белый
};

// Приватная функция парсинга Multiboot2
static void parse_multiboot2(uint64_t addr) {
    struct multiboot2_tag* tag;
    for (tag = (struct multiboot2_tag*)(addr + 8);
         tag->type != 0;
         tag = (struct multiboot2_tag*)((uint8_t*)tag + ((tag->size + 7) & ~7))) {
        if (tag->type == 8) {
            struct multiboot2_tag_framebuffer* fb_tag = (void*)tag;
            framebuffer = (volatile uint8_t*)fb_tag->framebuffer_addr;
            fb_width    = fb_tag->framebuffer_width;
            fb_height   = fb_tag->framebuffer_height;
            fb_pitch    = fb_tag->framebuffer_pitch;
            fb_bpp      = fb_tag->framebuffer_bpp;
            return;
        }
    }
}

// Инициализация VBE драйвера
void vbe_init(uint32_t magic, uint64_t mboot_addr) {
    if (magic != 0x36d76289) return;
    parse_multiboot2(mboot_addr);
}

// Установка пикселя
void vbe_put_pixel(uint32_t x, uint32_t y, uint8_t color) {
    if (x >= fb_width || y >= fb_height || color >= 16) return;
    if (fb_bpp == 4) {
        uint32_t byte_offset = y * fb_pitch + (x / 2);
        uint8_t shift = (x & 1) ? 0 : 4;
        uint8_t mask = (x & 1) ? 0x0F : 0xF0;
        framebuffer[byte_offset] = (framebuffer[byte_offset] & mask) | (color << shift);
    } else if (fb_bpp == 32) {
        uint32_t pixel_offset = y * fb_pitch + x * 4;
        uint32_t *pixel = (uint32_t*)(framebuffer + pixel_offset);
        *pixel = vga_palette[color];
    }
}

// Заполнение прямоугольника
void vbe_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t color) {
    for (uint32_t j = 0; j < height; j++) {
        for (uint32_t i = 0; i < width; i++) {
            vbe_put_pixel(x + i, y + j, color);
        }
    }
}

// Очистка экрана
void vbe_clear_screen(uint8_t color) {
    vbe_fill_rect(0, 0, fb_width, fb_height, color);
}  

#endif // _ALPHIX_DRIVERS_OUTPUT_VBE_H