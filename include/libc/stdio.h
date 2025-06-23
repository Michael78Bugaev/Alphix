// Minimal stdio for Alphix userland
// include/libc/stdio.h

#ifndef _STDIO_H
#define _STDIO_H

#include <libc/unistd.h>

#define EOF (-1)

static inline int putchar(int c)
{
    unsigned char ch = (unsigned char)c;
    return write(1, &ch, 1) == 1 ? c : EOF;
}

static inline int getchar(void)
{
    unsigned char ch;
    return read(0, &ch, 1) == 1 ? (int)ch : EOF;
}

static inline int puts(const char *s)
{
    int len = 0;
    while (s[len]) len++;
    if (write(1, s, len) < 0) return EOF;
    if (write(1, "\n", 1) < 0) return EOF;
    return len + 1;
}

#endif // _STDIO_H 