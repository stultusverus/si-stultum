
#include "ssfn.h"

#include "conlib.h"

void conlib_init(ssfn_font_t *font, void *fbbase, unsigned int w,
                 unsigned int h, unsigned int p) {
  /* set up context by global variables */
  ssfn_src = font;       /* the bitmap font to use */
  ssfn_dst.ptr = fbbase; /* framebuffer address and bytes per line */
  ssfn_dst.w = w;
  ssfn_dst.h = h;
  ssfn_dst.x = 0; /* coordinates to draw to */
  ssfn_dst.y = 0;
  ssfn_dst.p = p;
  ssfn_dst.fg = 0xffffffff; /* colors, white on black */
  ssfn_dst.bg = 0;
}

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

int cstrlen(const char *str) {
  register const char *s;
  for (s = str; *s; ++s)
    ;
  return (s - str);
}

char *strrev(char *str) {
  int i;
  int j;
  unsigned char a;
  unsigned len = cstrlen((const char *)str);
  for (i = 0, j = len - 1; i < j; i++, j--) {
    a = str[i];
    str[i] = str[j];
    str[j] = a;
  }
  return str;
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
  cp += cstrlen(buff);
  *cp++ = '.';
  while (n--) {
    value *= 10;
    int digit = value;
    *cp++ = '0' + digit;
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

void set_bg(uint32_t color) { ssfn_dst.bg = color; }
void set_fg(uint32_t color) { ssfn_dst.fg = color; }
void set_color(uint32_t fg, uint32_t bg) {
  ssfn_dst.fg = fg;
  ssfn_dst.bg = bg;
}
uint32_t get_bg() { return ssfn_dst.bg; }
uint32_t get_fg() { return ssfn_dst.fg; }

void cls() {
  unsigned int len = ssfn_dst.w * ssfn_dst.h;
  register uint32_t *ptr = (uint32_t *)ssfn_dst.ptr;
  while (len-- > 0)
    *ptr++ = ssfn_dst.bg;
}
