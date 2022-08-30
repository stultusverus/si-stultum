#ifndef __CONLIB_H__
#define __CONLIB_H__

#define SSFN_CONSOLEBITMAP_TRUECOLOR
#include "ssfn.h"

int cputc(int ch);
int cputs(char *s);
void cinit(ssfn_font_t *font, void *fbbase, int w, int h, int p);
char *citoa(int value, char *str, int base);
char *citoaul(unsigned long value, char *str, int base);
char *cstrrev(char *str);
int cstrlen(const char *str);

#endif //__CONLIB_H__