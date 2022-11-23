#pragma once

#include <stdarg.h>
#include <stdint.h>

typedef int __putchar_func_ptr(int);

static inline int is_alpha(int c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static inline int is_digit(int c) { return c >= '0' && c <= '9'; }

static inline int chint(int c) { return is_digit(c) ? c - '0' : -1; }

int __proto_print(__putchar_func_ptr __putchar, const char *format, ...);