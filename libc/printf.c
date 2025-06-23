#include <libc/printf.h>
#include <libc/unistd.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

// Вспомогательная функция для вывода одного символа
static void out_char(char c, char **buf, size_t *rem, size_t *total) {
    if (*rem > 1) {
        **buf = c;
        (*buf)++;
        (*rem)--;
    }
    (*total)++;
}

int vsnprintf(char *str, size_t size, const char *fmt, va_list ap) {
    char *buf = str;
    size_t rem = size;
    size_t total = 0;
    while (*fmt) {
        if (*fmt != '%') {
            out_char(*fmt, &buf, &rem, &total);
            fmt++;
            continue;
        }
        fmt++;
        if (!*fmt) break;
        switch (*fmt) {
            case 'c': {
                char c = (char)va_arg(ap, int);
                out_char(c, &buf, &rem, &total);
                break;
            }
            case 's': {
                char *s = va_arg(ap, char *);
                if (!s) s = "(null)";
                while (*s) {
                    out_char(*s, &buf, &rem, &total);
                    s++;
                }
                break;
            }
            case 'd':
            case 'i': {
                int val = va_arg(ap, int);
                unsigned int u;
                if (val < 0) {
                    out_char('-', &buf, &rem, &total);
                    u = (unsigned int)(-val);
                } else {
                    u = (unsigned int)val;
                }
                char tmp[32];
                int pos = 0;
                if (u == 0) tmp[pos++] = '0';
                while (u) {
                    tmp[pos++] = '0' + (u % 10);
                    u /= 10;
                }
                while (pos) {
                    out_char(tmp[--pos], &buf, &rem, &total);
                }
                break;
            }
            case 'u': {
                unsigned int u = va_arg(ap, unsigned int);
                char tmp[32];
                int pos = 0;
                if (u == 0) tmp[pos++] = '0';
                while (u) {
                    tmp[pos++] = '0' + (u % 10);
                    u /= 10;
                }
                while (pos) {
                    out_char(tmp[--pos], &buf, &rem, &total);
                }
                break;
            }
            case 'x':
            case 'X': {
                unsigned int u = va_arg(ap, unsigned int);
                char tmp[32];
                int pos = 0;
                if (u == 0) tmp[pos++] = '0';
                while (u) {
                    int d = u & 0xF;
                    tmp[pos++] = d < 10 ? '0' + d : 'a' + (d - 10);
                    u >>= 4;
                }
                while (pos) {
                    out_char(tmp[--pos], &buf, &rem, &total);
                }
                break;
            }
            case '%': {
                out_char('%', &buf, &rem, &total);
                break;
            }
            default: {
                out_char('%', &buf, &rem, &total);
                out_char(*fmt, &buf, &rem, &total);
                break;
            }
        }
        fmt++;
    }
    if (rem > 0) {
        *buf = '\0';
    }
    return (int)total;
}

int snprintf(char *str, size_t size, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vsnprintf(str, size, fmt, ap);
    va_end(ap);
    return r;
}

int sprintf(char *str, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vsnprintf(str, (size_t)-1, fmt, ap);
    va_end(ap);
    return r;
}

int printf(const char *fmt, ...) {
    char buf[256];
    va_list ap;
    va_start(ap, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    write(1, buf, len);
    return len;
}

int dprintf(int fd, const char *fmt, ...) {
    char buf[256];
    va_list ap;
    va_start(ap, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    return write(fd, buf, len);
} 