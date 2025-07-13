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
#include <multiboot2.h>
#include <out/vga.h>
#include <kprintf.h>
#include <debug.h>
#include <cpu/tss.h>
#include <cpu/syscall.h>
#include <mem/paging.h>
#include <mem/heap.h>
#include <in/keyboard.h>
#include <cpu/apic.h>
#include <string.h>

// parse multiboot2 tags
void parse_multiboot2(uint64_t addr) {
    struct multiboot2_tag* tag;
    
    // skip first 8 bytes (structure size)
    for (tag = (struct multiboot2_tag*)(addr + 8);
         tag->type != 0;
         tag = (struct multiboot2_tag*)((uint8_t*)tag + ((tag->size + 7) & ~7))) {
        
        switch (tag->type) {
            case 8: { // Framebuffer
                kprintf("Framebuffer found\n");
                break;
            }
        }
    }
}

// kernel entry point
void kernel_main(uint32_t magic, uint64_t mboot_info) {
    // check multiboot2 magic number
    if (magic != 0x36d76289) {
        kprintf("panic: invalid magic number\nkernel ended.");
        // error: invalid magic number
        return;
    }
    
    parse_multiboot2(mboot_info);
    // initialize paging (identity map first 1gb)
    paging_init();

    gdt_init();
    tss_init();
    pic_remap(0x20, 0x28);
    // mask all irq except keyboard (irq1)
    for (uint8_t irq = 0; irq < 8; ++irq) {
        if (irq != 1) pic_set_mask(irq);
    }
    for (uint8_t irq = 8; irq < 16; ++irq) {
        pic_set_mask(irq);
    }

    idt_init();
    apic_init();
    // system calls via int 0x80
    syscall_init();

    // initialize vga
    kclear();

    // initialize heap
    heap_init();
    
    cpu_enable_interrupts();

    base_init();

    kprintf("testing kgetch\n");
    while (1) {
        kprintf("> ");
        char c = kgetch();
        if (c == 'q') {
            kprintf("Quitting...\n");
            break;
        }
        kprintf("%c", c);
    }
} 