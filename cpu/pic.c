// Alphix kernel
// 8259A Programmable Interrupt Controller helpers
// cpu/pic.c

#include <cpu/pic.h>
#include <cpu/cpu.h>

#define ICW1_ICW4   0x01    // ICW4 (not) needed
#define ICW1_SINGLE 0x02    // Single (cascade) mode
#define ICW1_INTERVAL4 0x04 // Call address interval 4 (8)
#define ICW1_LEVEL  0x08    // Level triggered (edge) mode
#define ICW1_INIT   0x10    // Initialization - required!

#define ICW4_8086   0x01    // 8086/88 mode
#define ICW4_AUTO   0x02    // Auto EOI
#define ICW4_BUF_SLAVE 0x08
#define ICW4_BUF_MASTER 0x0C
#define ICW4_SFNM   0x10

void pic_remap(int offset1, int offset2)
{
    uint8_t a1 = inb(PIC1_DATA); // сохранить маски
    uint8_t a2 = inb(PIC2_DATA);

    // начинаем инициализацию
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);

    // задаём новые векторы
    outb(PIC1_DATA, offset1);
    outb(PIC2_DATA, offset2);

    // указываем каскад
    outb(PIC1_DATA, 4); // сообщаем master, что slave на IRQ2 (0000 0100)
    outb(PIC2_DATA, 2); // сообщаем slave его каскадовый номер (0000 0010)

    // режим 8086
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    // возвращаем маски
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}

void pic_send_eoi(uint8_t irq)
{
    if (irq >= 8)
        outb(PIC2_COMMAND, PIC_EOI);
    outb(PIC1_COMMAND, PIC_EOI);
}

void pic_set_mask(uint8_t irq_line)
{
    uint16_t port = (irq_line < 8) ? PIC1_DATA : PIC2_DATA;
    uint8_t value = inb(port) | (1 << (irq_line & 7));
    outb(port, value);
}

void pic_clear_mask(uint8_t irq_line)
{
    uint16_t port = (irq_line < 8) ? PIC1_DATA : PIC2_DATA;
    uint8_t value = inb(port) & ~(1 << (irq_line & 7));
    outb(port, value);
} 