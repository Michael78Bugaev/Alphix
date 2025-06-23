// Alphix kernel
// Built-in CPU functions
// include/cpu/cpu.h

#ifndef _ALPHIX_CPU_H
#define _ALPHIX_CPU_H

#include <stdint.h>
#include <stddef.h>

// Структура контекста прерывания
typedef struct {
    uint64_t int_no;      // номер вектора прерывания
    uint64_t err_code;    // код ошибки (или 0)
    // Сохранённые регистры CPU
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rdi, rsi, rbp, rbx, rdx, rcx, rax;
    // Сохранённые аппаратные значения
    uint64_t rip, cs, rflags, rsp, ss;
} cpu_registers_t;

// CPUID информация
typedef struct {
    char vendor[13];
    uint32_t max_basic;
    uint32_t max_extended;
    uint32_t features_edx;
    uint32_t features_ecx;
} cpu_info_t;

// Инициализация CPU
void cpu_init(void);

// Получение информации о CPU
cpu_info_t* cpu_get_info(void);

// Управление прерываниями
void cpu_enable_interrupts(void);
void cpu_disable_interrupts(void);

// Остановка CPU
void cpu_halt(void);

// Чтение/запись MSR
uint64_t cpu_read_msr(uint32_t msr);
void cpu_write_msr(uint32_t msr, uint64_t value);

// Работа с портами ввода-вывода
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
uint32_t inl(uint16_t port);

void outb(uint16_t port, uint8_t value);
void outw(uint16_t port, uint16_t value);
void outl(uint16_t port, uint32_t value);

// Чтение/запись регистра CR3
uint64_t cpu_read_cr3(void);
void cpu_write_cr3(uint64_t value);

char get_acsii_low(char code);
char get_acsii_high(char code);

#endif // _ALPHIX_CPU_H 