#include <string.h>

void *memset(void *s, int c, size_t n) {
    unsigned char *p = (unsigned char *)s;
    while (n--) {
        *p++ = (unsigned char)c;
    }
    return s;
}

void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    if (d < s) {
        while (n--) {
            *d++ = *s++;
        }
    } else if (d > s) {
        d += n;
        s += n;
        while (n--) {
            *--d = *--s;
        }
    }
    return dest;
}

size_t strlen(const char *s) {
    size_t len = 0;
    while (s[len]) {
        ++len;
    }
    return len;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *a = (const unsigned char *)s1;
    const unsigned char *b = (const unsigned char *)s2;
    while (n--) {
        if (*a != *b) {
            return *a - *b;
        }
        ++a;
        ++b;
    }
    return 0;
}

char *strcpy(char *dest, const char *src) {
    char *ret = dest;
    while ((*dest++ = *src++)) {
        ;
    }
    return ret;
} 

char *join(char *buf, char c) {
    size_t len = strlen(buf);
    buf[len] = c;
    buf[len + 1] = '\0';
    return buf;
}

void strnone(char *buf) {
    if (buf) buf[0] = '\0';
}

char *strins(char *buf, char c, size_t pos)
{
    size_t len = strlen(buf);
    if (pos > len) pos = len;
    memmove(buf + pos + 1, buf + pos, len - pos + 1); // include null
    buf[pos] = c;
    return buf;
}

char *strdel(char *buf, size_t pos)
{
    size_t len = strlen(buf);
    if (pos >= len) return buf;
    memmove(buf + pos, buf + pos + 1, len - pos);
    return buf;
}