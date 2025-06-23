// Minimal printf support for Alphix userland
// include/libc/printf.h

#ifndef _PRINTF_H
#define _PRINTF_H

#include <stdarg.h>
#include <stddef.h>

// Форматирование строки
int vsnprintf(char *str, size_t size, const char *fmt, va_list ap);
int snprintf(char *str, size_t size, const char *fmt, ...);
int sprintf(char *str, const char *fmt, ...);

// Вывод в stdout
int printf(const char *fmt, ...);

// Вывод в произвольный file descriptor
int dprintf(int fd, const char *fmt, ...);

#endif // _PRINTF_H 