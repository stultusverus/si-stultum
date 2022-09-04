#include "ssfn.h"

/***** renderer implementations *****/

/*** these go for both renderers ***/
#define SSFN_COMMON

/**
 * Error code strings
 */
const char *ssfn_errstr[] = {"",
                             "Memory allocation error",
                             "Bad file format",
                             "No font face found",
                             "Invalid input value",
                             "Invalid style",
                             "Invalid size",
                             "Glyph not found"};

/**
 * Decode an UTF-8 multibyte, advance string pointer and return UNICODE. Watch
 * out, no input checks
 *
 * @param **s pointer to an UTF-8 string pointer
 * @return unicode, and *s moved to next multibyte sequence
 */
uint32_t ssfn_utf8(char **s) {
  uint32_t c = **s;

  if ((**s & 128) != 0) {
    if (!(**s & 32)) {
      c = ((**s & 0x1F) << 6) | (*(*s + 1) & 0x3F);
      *s += 1;
    } else if (!(**s & 16)) {
      c = ((**s & 0xF) << 12) | ((*(*s + 1) & 0x3F) << 6) | (*(*s + 2) & 0x3F);
      *s += 2;
    } else if (!(**s & 8)) {
      c = ((**s & 0x7) << 18) | ((*(*s + 1) & 0x3F) << 12) |
          ((*(*s + 2) & 0x3F) << 6) | (*(*s + 3) & 0x3F);
      *s += 3;
    } else
      c = 0;
  }
  (*s)++;
  return c;
}

/**
 * public variables to configure
 */
ssfn_font_t *ssfn_src; /* font buffer with an inflated bitmap font */
ssfn_buf_t ssfn_dst;   /* destination frame buffer */

/**
 * Minimal OS kernel console renderer
 *
 * @param unicode character
 * @return error code
 */
int ssfn_putc(uint32_t unicode) {
  register uint32_t *o, *p;
  register uint8_t *ptr, *chr = NULL, *frg;
  register int i, j, k, l, m, y = 0, w, s = ssfn_dst.p / sizeof(uint32_t);

  if (!ssfn_src || ssfn_src->magic[0] != 'S' || ssfn_src->magic[1] != 'F' ||
      ssfn_src->magic[2] != 'N' || ssfn_src->magic[3] != '2' || !ssfn_dst.ptr ||
      !ssfn_dst.p)
    return SSFN_ERR_INVINP;
  w = ssfn_dst.w < 0 ? -ssfn_dst.w : ssfn_dst.w;
  for (ptr = (uint8_t *)ssfn_src + ssfn_src->characters_offs, i = 0;
       i < 0x110000; i++) {
    if (ptr[0] == 0xFF) {
      i += 65535;
      ptr++;
    } else if ((ptr[0] & 0xC0) == 0xC0) {
      j = (((ptr[0] & 0x3F) << 8) | ptr[1]);
      i += j;
      ptr += 2;
    } else if ((ptr[0] & 0xC0) == 0x80) {
      j = (ptr[0] & 0x3F);
      i += j;
      ptr++;
    } else {
      if ((uint32_t)i == unicode) {
        chr = ptr;
        break;
      }
      ptr += 6 + ptr[1] * (ptr[0] & 0x40 ? 6 : 5);
    }
  }
  if (!chr)
    return SSFN_ERR_NOGLYPH;
  ptr = chr + 6;
  o = (uint32_t *)(ssfn_dst.ptr + ssfn_dst.y * ssfn_dst.p +
                   ssfn_dst.x * sizeof(uint32_t));
  for (i = 0; i < chr[1]; i++, ptr += chr[0] & 0x40 ? 6 : 5) {
    if (ptr[0] == 255 && ptr[1] == 255)
      continue;
    frg = (uint8_t *)ssfn_src +
          (chr[0] & 0x40
               ? ((ptr[5] << 24) | (ptr[4] << 16) | (ptr[3] << 8) | ptr[2])
               : ((ptr[4] << 16) | (ptr[3] << 8) | ptr[2]));
    if ((frg[0] & 0xE0) != 0x80)
      continue;
    if (ssfn_dst.bg) {
      for (; y < ptr[1] && (!ssfn_dst.h || ssfn_dst.y + y < ssfn_dst.h);
           y++, o += s) {
        for (p = o, j = 0; j < chr[2] && (!w || ssfn_dst.x + j < w); j++, p++)
          *p = ssfn_dst.bg;
      }
    } else {
      o += (int)(ptr[1] - y) * s;
      y = ptr[1];
    }
    k = ((frg[0] & 0x1F) + 1) << 3;
    j = frg[1] + 1;
    frg += 2;
    for (m = 1; j && (!ssfn_dst.h || ssfn_dst.y + y < ssfn_dst.h);
         j--, y++, o += s)
      for (p = o, l = 0; l < k; l++, p++, m <<= 1) {
        if (m > 0x80) {
          frg++;
          m = 1;
        }
        if (ssfn_dst.x + l >= 0 && (!w || ssfn_dst.x + l < w)) {
          if (*frg & m) {
            *p = ssfn_dst.fg;
          } else if (ssfn_dst.bg) {
            *p = ssfn_dst.bg;
          }
        }
      }
  }
  if (ssfn_dst.bg)
    for (; y < chr[3] && (!ssfn_dst.h || ssfn_dst.y + y < ssfn_dst.h);
         y++, o += s) {
      for (p = o, j = 0; j < chr[2] && (!w || ssfn_dst.x + j < w); j++, p++)
        *p = ssfn_dst.bg;
    }
  ssfn_dst.x += chr[4];
  ssfn_dst.y += chr[5];
  return SSFN_OK;
}
