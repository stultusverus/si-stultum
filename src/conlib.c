
#include "ssfn.h"

#include "conlib.h"
#include "string.h"

extern void conlib_init(ssfn_font_t *font, void *fbbase, unsigned int w,
                        unsigned int h, unsigned int p);

extern void set_bg(uint32_t color);
extern void set_fg(uint32_t color);
extern void set_color(uint32_t fg, uint32_t bg);
extern uint32_t get_bg();
extern uint32_t get_fg();

int putc(int ch) {
  if (ch == '\n') {
    ssfn_dst.y += ssfn_src->height;
    ssfn_dst.x = 0;
  } else if (ch == '\r') {
    ssfn_dst.x = 0;
  } else if (ch == '\t') {
    // TODO: print \t.
  } else {
    ssfn_putc(ch);
  }
  if (ssfn_dst.x >= ssfn_dst.w) {
    ssfn_dst.y += ssfn_src->height;
    ssfn_dst.x = 0;
  }
  return ch;
}

int puts(const char *s) {
  while (*s)
    putc(*s++);
  return 0;
}

int putln(const char *s) {
  puts(s);
  putc('\n');
  return 0;
}

char *itoas(long value, char *buff, int base) {
  if (value < 0) {
    itoa(-value, buff + 1, base);
    buff[0] = '-';
    return buff;
  } else {
    return itoa(value, buff, base);
  }
}

char *itoa(unsigned long value, char *buff, int base) {
  unsigned long sum = value;
  int digit;
  int i = 0;
  do {
    digit = sum % base;
    if (digit < 0XA)
      buff[i++] = '0' + digit;
    else
      buff[i++] = 'A' + digit - 0xA;
    sum /= base;
  } while (sum);
  buff[i] = '\0';
  return strrev(buff);
}

char *ftoa(double value, char *buff) { return ftoan(value, buff, 8); }

char *ftoan(double value, char *buff, int n) {
  long int_part;
  char *cp = buff;
  int_part = value;
  value -= int_part;
  itoas(int_part, buff, 10);
  if (value < 0)
    value = -value;
  cp += strlen(buff);
  *cp++ = '.';
  while (n--) {
    value *= 10;
    int digit = value;
    *cp++     = '0' + digit;
    value -= digit;
  }
  *cp = '\0';
  return buff;
}

void puts_n(uint64_t n, ...) {
  va_list args;
  va_start(args, n);
  while (n--) {
    char *crt = va_arg(args, char *);
    puts(crt);
  }
  va_end(args);
}

void putsx(uint64_t n, uint32_t fg, uint32_t bg, ...) {
  uint32_t old_fg = get_fg();
  uint32_t old_bg = get_bg();
  set_color(fg, bg);

  va_list args;
  va_start(args, bg);
  while (n--) {
    char *crt = va_arg(args, char *);
    puts(crt);
  }

  va_end(args);
  set_color(old_fg, old_bg);
}

void puts_at(int x, int y, char *str) {
  int old_x = ssfn_dst.x;
  int old_y = ssfn_dst.y;
  while (x < 0)
    x += ssfn_dst.w / ssfn_src->width;
  while (y < 0)
    y += ssfn_dst.h / ssfn_src->height;
  ssfn_dst.x = x * ssfn_src->width;
  ssfn_dst.y = y * ssfn_src->height;
  puts(str);
  ssfn_dst.x = old_x;
  ssfn_dst.y = old_y;
}

void cls() {
  unsigned int len       = ssfn_dst.w * ssfn_dst.h;
  register uint32_t *ptr = (uint32_t *)ssfn_dst.ptr;
  while (len-- > 0)
    *ptr++ = ssfn_dst.bg;
}
