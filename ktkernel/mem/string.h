#pragma once
#include <cstddef>

extern "C"
{
size_t strlen(const char* s);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
char* strncpy(char* dest, const char* src, size_t n);
char* strcat(char* dest, const char* src);
}
