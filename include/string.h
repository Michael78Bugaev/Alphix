#ifndef _ALPHIX_STRING_H
#define _ALPHIX_STRING_H

#include <stddef.h>

void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
size_t strlen(const char *s);
int memcmp(const void *s1, const void *s2, size_t n);
char *strcpy(char *dest, const char *src);
char *join(char *buf, char c);
void strnone(char *buf);
char *strins(char *buf, char c, size_t pos);
char *strdel(char *buf, size_t pos);

#endif // _ALPHIX_STRING_H 