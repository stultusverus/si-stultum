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

char *strtok(char *s, const char *delim) {
  char *spanp;
  int c, sc;
  char *tok;
  static char *last;

  if (s == NULL && (s = last) == NULL)
    return NULL;

cont:
  c = *s++;
  for (spanp = (char *)delim; (sc = *spanp++) != 0;) {
    if (c == sc)
      goto cont;
  }

  if (c == 0) { /* no non-delimiter characters */
    last = NULL;
    return (NULL);
  }
  tok = s - 1;

  for (;;) {
    c     = *s++;
    spanp = (char *)delim;
    do {
      if ((sc = *spanp++) == c) {
        if (c == 0)
          s = NULL;
        else
          s[-1] = 0;
        last = s;
        return (tok);
      }
    } while (sc != 0);
  }

  return 0;
}
