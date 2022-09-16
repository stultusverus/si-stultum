#ifndef __CONLIB_H__
#define __CONLIB_H__

#include <stdarg.h>
#include <stdint.h>
#define SSFN_CONSOLEBITMAP_TRUECOLOR
#include "ssfn.h"

#define WHITE 0xffffffff
#define YELLOW 0x00fbf305
#define ORANGE 0x00ff6403
#define RED 0x00dd0907
#define MAGENTA 0x00f20884
#define BLUE 0x000000d3
#define PURPLE 0x004700a5
#define CYAN 0x0002abea
#define GREEN 0x001fb714
#define DARKGREEN 0x00006412
#define BROWN 0x00562c05
#define TAN 0x0090713a
#define LIGHTGREY 0x00c0c0c0
#define MEDIUMGREY 0x00808080
#define DARKGREY 0x00404040
#define BLACK 0x00000000

void conlib_init(ssfn_font_t *font, void *fbbase, unsigned int w,
                 unsigned int h, unsigned int p);

int putc(int ch);
int puts(const char *s);
int putln(const char *s);
void puts_n(uint64_t n, ...);
void puts_at(int x, int y, char *str);

void set_bg(uint32_t color);
void set_fg(uint32_t color);
void set_color(uint32_t fg, uint32_t bg);
uint32_t get_bg();
uint32_t get_fg();
void cls();

char *itoas(long value, char *buff, int base);
char *itoa(unsigned long value, char *buff, int base);
char *ftoa(double value, char *buff);
char *ftoan(double value, char *buff, int n);

char *strrev(char *str);
int cstrlen(const char *str);

#endif //__CONLIB_H__