// Alphix kernel
// Global Descriptor Table (GDT) setup for x86_64
// cpu/gdt.c

#include <cpu/gdt.h>
#include <cpu/cpu.h>

// Длина GDT: нулевой дескриптор + код ядра + данные ядра + пользовательские данные + пользовательский код + TSS (2 слота)
#define GDT_ENTRIES 7

// Флаги
#define GDT_FLAG_GRANULARITY  0x8  // G
#define GDT_FLAG_32BIT        0x4  // D/B = 0 для 64-бит, но оставим 0
#define GDT_FLAG_LONGMODE     0x2  // L

// DPL
#define GDT_ACCESS_RING0      0x00
#define GDT_ACCESS_RING3      0x60

// Access byte bits
#define GDT_ACCESS_PRESENT    0x80
#define GDT_ACCESS_CODE       0x18 // Executable + Descriptor type (code/data)
#define GDT_ACCESS_DATA       0x10 // Descriptor type = 1 (data), executable = 0
#define GDT_ACCESS_RW         0x02 // Readable/Writable

static gdt_entry_t gdt[GDT_ENTRIES];
static gdt_descriptor_t gdtr;

static void gdt_set_entry(int idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
    gdt[idx].limit_low = (uint16_t)(limit & 0xFFFF);
    gdt[idx].base_low  = (uint16_t)(base & 0xFFFF);
    gdt[idx].base_mid  = (uint8_t)((base >> 16) & 0xFF);
    gdt[idx].access    = access;
    gdt[idx].flags_limit = (uint8_t)(((limit >> 16) & 0x0F) | (flags << 4));
    gdt[idx].base_high = (uint8_t)((base >> 24) & 0xFF);
}

void gdt_init(void)
{
    // Заполнение GDTR
    gdtr.size = sizeof(gdt) - 1;
    gdtr.offset = (uint64_t)&gdt;

    // Нулевая запись
    gdt_set_entry(0, 0, 0, 0, 0);

    // Код сегмент ядра: base 0, limit 0, access: present | ring0 | code | readable, flags: granularity 4K | long mode
    gdt_set_entry(1, 0, 0, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE | GDT_ACCESS_RW,
                  GDT_FLAG_GRANULARITY | GDT_FLAG_LONGMODE);

    // Сегмент данных ядра: base 0, limit 0, access: present | ring0 | data | writable, flags: granularity 4K
    gdt_set_entry(2, 0, 0, GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA | GDT_ACCESS_RW,
                  GDT_FLAG_GRANULARITY);

    // Сегмент данных пользователя (RPL=3)
    gdt_set_entry(3, 0, 0, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_DATA | GDT_ACCESS_RW,
                  GDT_FLAG_GRANULARITY);
    // Сегмент кода пользователя (RPL=3, 64-bit)
    gdt_set_entry(4, 0, 0, GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_CODE | GDT_ACCESS_RW,
                  GDT_FLAG_GRANULARITY | GDT_FLAG_LONGMODE);

    // TSS-дескриптор будет заполнен при вызове tss_init()

    // Загрузка GDTR
    __asm__ __volatile__("lgdt %0" : : "m"(gdtr));

    // Обновить сегментные регистры
    __asm__ __volatile__(
        "mov $0x10, %%ax\n"   // 0x10 это селектор данных (index 2) RPL=0
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        "pushq $0x08\n"      // селектор кода (index1)
        "lea 1f(%%rip), %%rax\n"
        "pushq %%rax\n"
        "lretq\n"            // долгий рет для смены CS
        "1:"
        : : : "rax"
    );
}

// Установка TSS-дескриптора (64-bit) в GDT: два слота idx и idx+1
void gdt_set_tss_entry(int idx, uint64_t base, uint32_t limit)
{
    // Старший дескриптор
    gdt[idx].limit_low    = (uint16_t)(limit & 0xFFFF);
    gdt[idx].base_low     = (uint16_t)(base & 0xFFFF);
    gdt[idx].base_mid     = (uint8_t)((base >> 16) & 0xFF);
    gdt[idx].access       = 0x89; // present, system, type=9 (available TSS)
    gdt[idx].flags_limit  = (uint8_t)(((limit >> 16) & 0x0F));
    gdt[idx].base_high    = (uint8_t)((base >> 24) & 0xFF);

    // Старший 8 байт
    gdt[idx+1].limit_low   = (uint16_t)((base >> 32) & 0xFFFF);
    gdt[idx+1].base_low    = (uint16_t)((base >> 48) & 0xFFFF);
    gdt[idx+1].base_mid    = 0;
    gdt[idx+1].access      = 0;
    gdt[idx+1].flags_limit = 0;
    gdt[idx+1].base_high   = 0;
} 