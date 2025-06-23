// Alphix kernel
// Interrupt Descriptor Table (IDT) setup for x86_64
// cpu/idt.c

#include <cpu/idt.h>
#include <cpu/gdt.h>
#include <cpu/pic.h>
#include <cpu/cpu.h>
#include <debug.h>

#define IDT_SIZE 256

idt_entry_t idt[IDT_SIZE];
idt_descriptor_t idtr;

// Глобальный массив обработчиков
void (*interrupt_handlers[IDT_SIZE])(cpu_registers_t*) = {0};

extern void *isr_stub_table[]; // Таблица адресов ASM-заглушек

static void idt_set_gate(uint8_t vector, uint64_t handler, uint8_t flags)
{
    idt[vector].offset_low  = (uint16_t)(handler & 0xFFFF);
    idt[vector].selector    = 0x08; // Кодовый сегмент ядра
    idt[vector].ist         = 0;
    idt[vector].type_attr   = flags;
    idt[vector].offset_mid  = (uint16_t)((handler >> 16) & 0xFFFF);
    idt[vector].offset_high = (uint32_t)((handler >> 32) & 0xFFFFFFFF);
    idt[vector].zero        = 0;
}

// Диспетчер, вызываемый ASM-stub
void isr_dispatch(cpu_registers_t* regs)
{
    uint8_t vec = (uint8_t)regs->int_no;
    switch (vec) {
        case 1:
            qemu_debug_printf("Timer interrupt\n");
            break;
        case 14:
            qemu_debug_printf("Page fault!\nEIP: %08X\n", regs->rip);
            break;
        case 33:
            qemu_debug_printf("Keyboard interrupt\n");
            break;
        default:
            qemu_debug_printf("Unknown interrupt: %d\n", vec);
    }
    if (interrupt_handlers[vec])
        interrupt_handlers[vec](regs);

    // EOI для аппаратных IRQ
    if (vec >= 32 && vec <= 47)
        pic_send_eoi(vec - 32);
}

void idt_init(void)
{
    for (int i = 0; i < IDT_SIZE; ++i) {
        // По умолчанию прерывания DPL=0
        uint8_t flags = 0x8E;
        // Системный вызов int 0x80: DPL=3
        if (i == 0x80) {
            flags = 0xEE;
        }
        idt_set_gate(i, (uint64_t)isr_stub_table[i], flags);
    }

    idtr.size = sizeof(idt) - 1;
    idtr.offset = (uint64_t)&idt;
    __asm__ __volatile__("lidt %0" : : "m"(idtr));
}

void idt_register_handler(uint8_t vector, void (*handler)(cpu_registers_t*))
{
    interrupt_handlers[vector] = handler;
} 

void idt_uninstall_handler(uint8_t vector)
{
    interrupt_handlers[vector] = NULL;
}