// Minimal POSIX unistd for Alphix userland
// include/libc/unistd.h

#ifndef _UNISTD_H
#define _UNISTD_H

#include <stdint.h>
#include <stddef.h>

static inline int read(int fd, void *buf, size_t count)
{
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "0"(0), "D"(fd), "S"(buf), "d"(count)
        : "memory"
    );
    return ret;
}

static inline int write(int fd, const void *buf, size_t count)
{
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "0"(1), "D"(fd), "S"(buf), "d"(count)
        : "memory"
    );
    return ret;
}

static inline void _exit(int status)
{
    asm volatile(
        "int $0x80"
        :
        : "a"(60), "D"(status)
        : "memory"
    );
    __builtin_unreachable();
}

#endif // _UNISTD_H 