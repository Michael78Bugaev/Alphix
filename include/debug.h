// debug.h - QEMU debug output utilities
#ifndef _ALPHIX_DEBUG_H
#define _ALPHIX_DEBUG_H

#include <stdint.h>
#include <stdarg.h>

// Вывести один символ в порт 0xE9
void qemu_debug_putc(char c);

// printf-подобный вывод в порт 0xE9 (поддерживает %c %s %u %d %x %% )
void qemu_debug_printf(const char *fmt, ...);

#endif // _ALPHIX_DEBUG_H 