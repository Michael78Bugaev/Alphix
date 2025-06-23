// Alphix kernel
// System call interface
// include/cpu/syscall.h

#ifndef _ALPHIX_SYSCALL_H
#define _ALPHIX_SYSCALL_H

#include <cpu/cpu.h>

// Инициализация механизма системных вызовов (int 0x80)
void syscall_init(void);

#endif // _ALPHIX_SYSCALL_H 