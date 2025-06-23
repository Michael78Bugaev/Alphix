// Alphix kernel
// Interrupt Descriptor Table (IDT) definitions for x86_64
// include/cpu/idt.h

#ifndef _ALPHIX_IDT_H
#define _ALPHIX_IDT_H

#include <stdint.h>
#include <cpu/cpu.h>

// Запись IDT (16 байт в 64-битном режиме)
typedef struct __attribute__((packed)) {
    uint16_t offset_low;    // Биты 0..15 адреса обработчика
    uint16_t selector;      // Селектор сегмента кода
    uint8_t  ist;           // Биты IST
    uint8_t  type_attr;     // Тип и атрибуты
    uint16_t offset_mid;    // Биты 16..31 адреса обработчика
    uint32_t offset_high;   // Биты 32..63 адреса обработчика
    uint32_t zero;          // Зарезервировано
} idt_entry_t;

// Описание IDTR
typedef struct __attribute__((packed)) {
    uint16_t size;
    uint64_t offset;
} idt_descriptor_t;

// Инициализация IDT
void idt_init(void);

// Регистрация C-обработчика для вектора
void idt_register_handler(uint8_t vector, void (*handler)(cpu_registers_t*));

void idt_uninstall_handler(uint8_t vector);

#endif // _ALPHIX_IDT_H 