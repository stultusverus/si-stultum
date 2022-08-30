
#include "ssfn.h"

#include "conlib.h"

void cinit(ssfn_font_t *font, void *fbbase, int w, int h, int p) {
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

int cputc(int ch) {
  if (ch == '\n') {
    ssfn_dst.y += ssfn_src->height;
    ssfn_dst.x = 0;
  } else if (ch == '\r') {
    ssfn_dst.x = 0;
  } else {
    ssfn_putc(ch);
  }
  if (ssfn_dst.x >= ssfn_dst.w) {
    ssfn_dst.y += ssfn_src->height;
    ssfn_dst.x = 0;
  }
  return ch;
}

int cputs(const char *s) {
  while (*s)
    cputc(*s++);
  return 0;
}

int cputln(const char *s) {
  cputs(s);
  cputc('\n');
  return 0;
}

char *citoa(long value, char *buff, int base) {
  if (value < 0) {
    citoaul(-value, buff + 1, base);
    buff[0] = '-';
    return buff;
  } else {
    return citoaul(value, buff, base);
  }
}

char *citoaul(unsigned long value, char *buff, int base) {
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
  return cstrrev(buff);
}

int cstrlen(const char *str) {
  register const char *s;
  for (s = str; *s; ++s)
    ;
  return (s - str);
}

char *cstrrev(char *str) {
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

char *cftoa(double value, char *buff) { return cftoan(value, buff, 8); }

char *cftoan(double value, char *buff, int n) {
  long int_part;
  char *cp = buff;
  int_part = value;
  value -= int_part;
  citoa(int_part, buff, 10);
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