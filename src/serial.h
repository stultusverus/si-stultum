#pragma once

#include "fmt.h"

#define serial_printf(fmt, ...)                                                \
  {                                                                            \
    __proto_print(kputc, "[%9d.%03d] ", ms_count / 1000, ms_count % 1000);     \
    __proto_print(kputc, fmt, ##__VA_ARGS__);                                  \
  }

int init_serial();

int serial_transmit_empty();
int kputc(int c);
int kputs(const char *s);

int serial_received();
int kgetc();