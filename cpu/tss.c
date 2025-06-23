// Alphix kernel
// TSS setup for x86_64
// cpu/tss.c

#include <cpu/tss.h>
#include <cpu/gdt.h>
#include <string.h>

// Статический стек ядра на уровне 0 (для обработки прерываний из ring3)
#define KERNEL_STACK_SIZE 8192
static uint8_t kernel_stack[KERNEL_STACK_SIZE] __attribute__((aligned(16)));

// Объект TSS
static tss_t tss_entry;

void tss_init(void)
{
    // Очистка TSS
    memset(&tss_entry, 0, sizeof(tss_entry));

    // Установка rsp0 на вершину kernel_stack
    tss_entry.rsp0 = (uint64_t)(kernel_stack + KERNEL_STACK_SIZE);

    // Устанавливаем TSS-дескриптор в GDT на индекс 5
    extern void gdt_set_tss_entry(int idx, uint64_t base, uint32_t limit);
    gdt_set_tss_entry(5, (uint64_t)&tss_entry, sizeof(tss_entry) - 1);

    // Загружаем TR селектором 5*8 = 0x28
    __asm__ __volatile__("ltr %%ax" :: "a"((uint16_t)(5 * 8)));
} 