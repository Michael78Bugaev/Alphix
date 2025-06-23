// Alphix kernel
// Kernel printf interface
// include/cpu/kprintf.h

#ifndef _ALPHIX_KPRINTF_H
#define _ALPHIX_KPRINTF_H

#include <stdarg.h>

// Форматированный вывод в kernel (VBE terminal)
void kvprintf(const char *fmt, va_list ap);
void kprintf(const char *fmt, ...);
unsigned char *kscan();

#endif // _ALPHIX_KPRINTF_H 