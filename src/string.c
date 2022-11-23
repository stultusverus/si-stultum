#include "string.h"

unsigned long strlen(const char *str) {
  register const char *s;
  for (s = str; *s; ++s)
    ;
  return (s - str);
}

char *strrev(char *str) {
  int i;
  int j;
  unsigned char a;
  unsigned len = strlen((const char *)str);
  for (i = 0, j = len - 1; i < j; i++, j--) {
    a      = str[i];
    str[i] = str[j];
    str[j] = a;
  }
  return str;
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 == *s2++)
    if (*s1++ == 0)
      return (0);
  return (*(unsigned char *)s1 - *(unsigned char *)--s2);
}