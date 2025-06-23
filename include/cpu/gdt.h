// Alphix kernel
// Global Descriptor Table (GDT) definitions for x86_64
// include/cpu/gdt.h

#ifndef _ALPHIX_GDT_H
#define _ALPHIX_GDT_H

#include <stdint.h>

// Структура одной записи GDT (8 байт)
typedef struct __attribute__((packed)) {
    uint16_t limit_low;      // Биты 0..15 лимита
    uint16_t base_low;       // Биты 0..15 базы
    uint8_t  base_mid;       // Биты 16..23 базы
    uint8_t  access;         // Байты доступа
    uint8_t  flags_limit;    // 4 бита лимита (16..19) + 4 бита флагов
    uint8_t  base_high;      // Биты 24..31 базы
} gdt_entry_t;

// Описание GDTR
typedef struct __attribute__((packed)) {
    uint16_t size;   // Размер таблицы - 1
    uint64_t offset; // Линейный адрес GDT
} gdt_descriptor_t;

// Инициализация GDT
void gdt_init(void);

// Установка TSS-дескриптора (64-bit) в GDT: два слота idx и idx+1
void gdt_set_tss_entry(int idx, uint64_t base, uint32_t limit);

#endif // _ALPHIX_GDT_H 