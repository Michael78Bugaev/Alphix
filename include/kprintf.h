// Alphix kernel
// Kernel printf interface
// include/cpu/kprintf.h

#ifndef _ALPHIX_KPRINTF_H
#define _ALPHIX_KPRINTF_H

#include <stdarg.h>

// Форматированный вывод в kernel (VGA terminal)
void kvprintf(const char *fmt, va_list ap);
void kprintf(const char *fmt, ...);
void kscan(char *buf);

#endif // _ALPHIX_KPRINTF_H 