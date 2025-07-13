// Alphix kernel
// System call implementation via int 0x80
// cpu/syscall.c

#include <cpu/syscall.h>
#include <cpu/idt.h>
#include <cpu/cpu.h>
#include <in/keyboard.h>
#include <kprintf.h>

// sys_read: fd=0 -> stdin (keyboard buffer)
static uint64_t sys_read(uint64_t fd, char *buf, uint64_t count)
{
    if (fd == 0) {
        uint64_t i = 0;
        char c;
        for (; i < count; ++i) {
            // дождаться символа
            while (!keyboard_buffer_pop(&c)) {
                __asm__ __volatile__("hlt");
            }
            buf[i] = c;
        }
        return i;
    }
    return (uint64_t)-1;
}

// sys_write: fd=1,2 -> окно терминала
static uint64_t sys_write(uint64_t fd, const char *buf, uint64_t count)
{
    if (fd == 1 || fd == 2) {
        for (uint64_t i = 0; i < count; ++i) {
            kprintf("%c", buf[i]);
        }
        return count;
    }
    return (uint64_t)-1;
}

// sys_exit: завершение процесса (останавливает ядро)
static void sys_exit(uint64_t status)
{
    (void)status;
    cpu_disable_interrupts();
    for (;;) {
        cpu_halt();
    }
}

// Основной обработчик системных вызовов
static void syscall_handler(cpu_registers_t *regs)
{
    uint64_t num = regs->rax;
    switch (num) {
        case 0:
            regs->rax = sys_read(regs->rdi, (char*)regs->rsi, regs->rdx);
            break;
        case 1:
            regs->rax = sys_write(regs->rdi, (const char*)regs->rsi, regs->rdx);
            break;
        case 60:
            sys_exit(regs->rdi);
            break;
        default:
            regs->rax = (uint64_t)-1;
            break;
    }
}

void syscall_init(void)
{
    // Регистрируем обработчик на вектор 0x80
    idt_register_handler(0x80, syscall_handler);
} 