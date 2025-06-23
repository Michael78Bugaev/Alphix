// Alphix kernel
// Main kernel start file
// kernel/kernel.c

#include <stdint.h>
#include <alphix.h>
#include <cpu/cpu.h>
#include <cpu/gdt.h>
#include <cpu/idt.h>
#include <cpu/pic.h>
#include <stddef.h>
#include <out/vbe_terminal.h>
#include <multiboot2.h>
#include <out/vbe.h>
#include <kprintf.h>
#include <debug.h>
#include <cpu/tss.h>
#include <cpu/syscall.h>
#include <mem/paging.h>
#include <in/keyboard.h>
#include <cpu/apic.h>

// Парсинг тегов Multiboot2
void parse_multiboot2(uint64_t addr) {
    struct multiboot2_tag* tag;
    
    // Пропускаем первые 8 байт (размер структуры)
    for (tag = (struct multiboot2_tag*)(addr + 8);
         tag->type != 0;
         tag = (struct multiboot2_tag*)((uint8_t*)tag + ((tag->size + 7) & ~7))) {
        
        switch (tag->type) {
            case 8: { // Framebuffer
                struct multiboot2_tag_framebuffer* fb_tag = 
                    (struct multiboot2_tag_framebuffer*)tag;
                
                framebuffer = (uint8_t*)fb_tag->framebuffer_addr;
                fb_width = fb_tag->framebuffer_width;
                fb_height = fb_tag->framebuffer_height;
                fb_pitch = fb_tag->framebuffer_pitch;
                fb_bpp = fb_tag->framebuffer_bpp;
                break;
            }
        }
    }
}

void keyboard_handler(cpu_registers_t* regs);

// Точка входа ядра
void kernel_main(uint32_t magic, uint64_t mboot_info) {
    // Проверка магического числа Multiboot2
    if (magic != 0x36d76289) {
        // Ошибка: неверное магическое число
        return;
    }
    
    parse_multiboot2(mboot_info);
    // Инициализация Paging (identity map первых 1GB)
    paging_init();

    gdt_init();
    tss_init();
    pic_remap(0x20, 0x28);
    // Маскируем все IRQ, кроме клавиатуры (IRQ1)
    for (uint8_t irq = 0; irq < 8; ++irq) {
        if (irq != 1) pic_set_mask(irq);
    }
    for (uint8_t irq = 8; irq < 16; ++irq) {
        pic_set_mask(irq);
    }

    idt_init();
    apic_init();
    // Системные вызовы через int 0x80
    syscall_init();

    vbe_init(magic, mboot_info);

    vbe_terminal_init(magic, mboot_info, 7, 0);
    vbe_printf("Alphix kernel is running...\n");
    cpu_enable_interrupts();

    kprintf("Alphix simple shell\n");
    while (1) {
        kprintf("> ");
        char *buf = kscan();
        vbe_printf("You typed: %s\n", buf);
    }
} 

// Исправленный обработчик клавиатуры
void keyboard_handler(cpu_registers_t* regs) {
    uint8_t scancode = inb(0x60);
    // push only make-codes
    if (!(scancode & 0x80)) {
        char c = get_acsii_low(scancode);
        if (c) {
            if (c == '\n') {
                vbe_terminal_putchar('\n');
            } else {
                keyboard_buffer_push(c);
                // echo to terminal
                vbe_terminal_putchar(c);
            }
        }
    }
}