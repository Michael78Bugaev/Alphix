// Alphix kernel
// Local APIC and timer setup
// cpu/apic.c

#include <cpu/apic.h>
#include <cpu/cpu.h>
#include <cpu/idt.h>
#include <out/vga.h>

// APIC register offsets
#define APIC_OFFSET_ID        0x020
#define APIC_OFFSET_EOI       0x0B0
#define APIC_OFFSET_SVR       0x0F0
#define APIC_OFFSET_LVT_TIMER 0x320
#define APIC_OFFSET_TIMER_ICR 0x380
#define APIC_OFFSET_TIMER_DIV 0x3E0

// Timer vector
#define APIC_TIMER_VECTOR 0xEF

static volatile uint32_t *apic = NULL;
// Счетчик для замедления мигания курсора
static uint32_t blink_counter = 0;
// Интервал мигания (в количествах APIC interrupts)
#define BLINK_INTERVAL 20

// Read MSR APIC base and map pointer
static void apic_map(void)
{
    uint64_t msr = cpu_read_msr(0x1B);
    uint64_t base = msr & 0xFFFFF000;
    apic = (volatile uint32_t *)base;
}

void apic_init(void)
{
    apic_map();

    // Enable APIC (set Spurious Interrupt Vector Register bit 8)
    apic[APIC_OFFSET_SVR/4] |= 0x100;

    // Set timer divide config to divide by 16
    apic[APIC_OFFSET_TIMER_DIV/4] = 0x3;

    // LVT Timer: vector, periodic mode (bit 17 = 1)
    apic[APIC_OFFSET_LVT_TIMER/4] = APIC_TIMER_VECTOR | (1 << 17);

    // Initial count for timer (example value)
    apic[APIC_OFFSET_TIMER_ICR/4] = 0x100000;

    // Register handler in IDT
    idt_register_handler(APIC_TIMER_VECTOR, apic_handler);
}

void apic_handler(cpu_registers_t *regs)
{
    (void)regs;
    // Мигаем курсором с заданным интервалом
    if (++blink_counter >= BLINK_INTERVAL) {
        blink_counter = 0;
        // Для VGA используем аппаратный курсор - просто получаем текущую позицию
        uint16_t current_pos = get_cursor();
        // Просто переустанавливаем курсор в ту же позицию для мигания
        set_cursor(current_pos);
    }

    // Send EOI to APIC
    apic[APIC_OFFSET_EOI/4] = 0;
} 