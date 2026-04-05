#include "mem/string.h"

extern "C"
{
size_t strlen(const char* s)
{
    size_t len = 0;
    while (s[len])
        len++;

    return len;
}

int strcmp(const char* s1, const char* s2)
{
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, size_t n)
{
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0)
        return 0;

    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

char* strncpy(char* dest, const char* src, size_t n)
{
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];

    for (; i < n; i++)
        dest[i] = '\0';

    return dest;
}

char* strcat(char* dest, const char* src)
{
    char* tmp = dest;
    while (*dest)
        dest++;

    while ((*dest++ = *src++) != '\0')
        ;

    return tmp;
}
}
