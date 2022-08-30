#ifndef __CONLIB_H__
#define __CONLIB_H__

#define SSFN_CONSOLEBITMAP_TRUECOLOR
#include "ssfn.h"

int cputc(int ch);
int cputs(char *s);
int cputln(char *s);
void cinit(ssfn_font_t *font, void *fbbase, int w, int h, int p);
char *citoa(long value, char *buff, int base);
char *citoaul(unsigned long value, char *buff, int base);
char *cstrrev(char *str);
int cstrlen(const char *str);
char *cftoa(double value, char *buff);
char *cftoan(double value, char *buff, int n);

#endif //__CONLIB_H__