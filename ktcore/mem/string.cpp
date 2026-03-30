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

char* strcpy(char* dest, const char* src)
{
    char* tmp = dest;
    while ((*dest++ = *src++) != '\0')
        ;
    return tmp;
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

char* strcat(char* dest, const char* src)
{
    char* tmp = dest;
    while (*dest)
        dest++;

    while ((*dest++ = *src++) != '\0')
        ;

    return tmp;
}

char* strchr(const char* s, int c)
{
    while (*s != (char)c)
        if (!*s++)
            return NULL;

    return (char*)s;
}

char* strrchr(const char* s, int c)
{
    char* last = NULL;
    do {
        if (*s == (char)c)
            last = (char*)s;
    } while (*s++);
    return last;
}

char* strstr(const char* haystack, const char* needle)
{
    size_t n = strlen(needle);
    if (!n)
        return (char*)haystack;

    while (*haystack) {
        if (!strncmp(haystack, needle, n))
            return (char*)haystack;
        haystack++;
    }
    return NULL;
}
}
