// Alphix kernel
// Local APIC interface
// include/cpu/apic.h

#ifndef _ALPHIX_APIC_H
#define _ALPHIX_APIC_H

#include <cpu/cpu.h>

// Инициализация локального APIC и таймера
void apic_init(void);
// Обработчик APIC-таймера
void apic_handler(cpu_registers_t *regs);

#endif // _ALPHIX_APIC_H 