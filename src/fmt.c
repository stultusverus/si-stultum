#include "fmt.h"
#include "string.h"

typedef int __putchar_func_ptr(int);

char __str_buff[32];

static const char *__read_int(const char *p, int *ret) {
  *ret = 0;
  while (is_digit(*p)) {
    *ret = *ret * 10 + chint(*p);
    p++;
  }
  return p;
}

char *__utoa(uint64_t value, char *buff, int base) {
  int digit;
  int i = 0;
  do {
    digit = value % base;
    if (digit < 0XA)
      buff[i++] = '0' + digit;
    else
      buff[i++] = 'A' + digit - 0xA;
    value /= base;
  } while (value);
  buff[i] = '\0';
  return strrev(buff);
}

char *__itoa(int64_t value, char *buff, int base) {
  if (value < 0) {
    __utoa(-value, buff + 1, base);
    buff[0] = '-';
    return buff;
  } else {
    return __utoa(value, buff, base);
  }
}

// __print_int can only print decimal.
static int __print_int(__putchar_func_ptr __putchar, long long int n, int width,
                       int precision, int align, int prefix, int pad) {
  int num_len = 0;
  int cnt     = 0;
  if (precision != 0 || n != 0) {
    if (n < 0) {
      prefix = '-';
      n      = -n;
    }
    do {
      int digit             = n % 10;
      __str_buff[num_len++] = '0' + digit;
      n                     = n / 10;
    } while (n);
    if (precision > 0)
      pad = ' ';
    while (precision > num_len)
      __str_buff[num_len++] = '0';
  }
  __str_buff[num_len] = '\0';
  strrev(__str_buff);

  width -= num_len;
  if (align) { // left
    if (prefix) {
      __putchar(prefix);
      cnt++;
      width--;
    }
    for (int i = 0; __str_buff[i]; i++)
      __putchar(__str_buff[i]);
    cnt += num_len;
    while (width-- > 0) {
      __putchar(' ');
      cnt++;
    }
  } else { // right
    if (prefix) {
      if (pad == '0') {
        __putchar(prefix);
        cnt++;
      }
      width--;
    }
    while (width-- > 0) {
      __putchar(pad);
      cnt++;
    }
    if (prefix && pad != '0') {
      __putchar(prefix);
      cnt++;
    }
    for (int i = 0; __str_buff[i]; i++)
      __putchar(__str_buff[i]);
    cnt += num_len;
  }
  return cnt;
}

// __print_unsigned can print more than dec thus more args.
static int __print_unsigned(__putchar_func_ptr __putchar, unsigned long long n,
                            int width, int precision, int align, int pad,
                            int base, int upper) {
  int len = 0;
  int cnt = 0;
  if (precision != 0 || n != 0) {
    do {
      int digit = n % base;
      if (digit < 0xa)
        __str_buff[len++] = '0' + digit;
      else
        __str_buff[len++] = (upper ? 'A' : 'a') + digit - 0xa;
      n = n / base;
    } while (n);
    if (precision > 0)
      pad = ' ';
    while (precision > len)
      __str_buff[len++] = '0';
  }
  __str_buff[len] = '\0';
  strrev(__str_buff);

  width -= len;
  if (align) { // left
    for (int i = 0; __str_buff[i]; i++)
      __putchar(__str_buff[i]);
    cnt += len;
    while (width-- > 0) {
      __putchar(' ');
      cnt++;
    }
  } else { // right
    while (width-- > 0) {
      __putchar(pad);
      cnt++;
    }
    for (int i = 0; __str_buff[i]; i++)
      __putchar(__str_buff[i]);
    cnt += len;
  }
  return cnt;
}

static int __print_char(__putchar_func_ptr __putchar, char c, int width,
                        int align) {
  if (align) // left
    __putchar(c);
  for (int i = 0; i < width - 1; i++)
    __putchar(' ');
  if (!align) // right
    __putchar(c);
  return width;
}

static int __print_str(__putchar_func_ptr __putchar, char *s, int width,
                       int precision, int align) {
  int ret     = 0;
  int str_len = strlen(s);
  if (precision >= 0 && str_len > precision)
    str_len = precision;
  int pad_len = width > str_len ? width - str_len : 0;
  ret         = str_len + pad_len;

  if (!align)
    for (int i = 0; i < pad_len; i++)
      __putchar(' ');

  for (int i = 0; i < str_len; i++)
    __putchar(s[i]);

  if (align)
    for (int i = 0; i < pad_len; i++)
      __putchar(' ');

  return ret;
}

static int __is_fmtflag(int c) {
  const char *fmtflags = "-+ 0";
  for (int i = 0; fmtflags[i]; i++)
    if (c == fmtflags[i])
      return 1;
  return 0;
}

int __proto_print(__putchar_func_ptr __putchar, const char *format, ...) {
  va_list ap;
  int ret = 0;
  char ch;
  va_start(ap, format);
  while ((ch = *format++) != 0) {
    if (ch == '%') {
      ch        = *format++;
      int width = -1, precision = -1,
          length = 0; // length < 0 for h, > 0 for l.
      int align  = 0; // 0 for right 1 for left
      int prefix = 0; // prefix character, 0 for none
      int pad    = ' ';
      while (__is_fmtflag(ch)) {
        switch (ch) {
        case '-':
          align = 1;
          break;
        case '+':
          prefix = '+';
          break;
        case ' ':
          if (prefix != '+')
            prefix = ' ';
          break;
        case '0':
          pad = '0';
          break;
          // default:
        }
        ch = *format++;
      }

      if (is_digit(ch)) { // width
        format = __read_int(format - 1, &width);
        ch     = *format++;
      }
      if (ch == '.') {
        format = __read_int(format, &precision);
        ch     = *format++;
      }
      if (ch == 'h' || ch == 'l') {
        length = ch == 'h' ? -1 : 1;
        if (*format == ch) {
          length *= 2;
          format++;
        }
        ch = *format++;
      }
      switch (ch) {
      case 'd': // fall-through
      case 'i':
        switch (length) {
        case -2: {
          signed char val = va_arg(ap, int);
          ret +=
              __print_int(__putchar, val, width, precision, align, prefix, pad);
          break;
        }
        case -1: {
          short val = va_arg(ap, int);
          ret +=
              __print_int(__putchar, val, width, precision, align, prefix, pad);
          break;
        }
        case 0: {
          int val = va_arg(ap, int);
          ret +=
              __print_int(__putchar, val, width, precision, align, prefix, pad);
          break;
        }
        case 1: {
          long val = va_arg(ap, long);
          ret +=
              __print_int(__putchar, val, width, precision, align, prefix, pad);
          break;
        }
        case 2: {
          long long val = va_arg(ap, long long);
          ret +=
              __print_int(__putchar, val, width, precision, align, prefix, pad);
          break;
        }
        }
        break;
      case 'b':
      case 'u':
      case 'o':
      case 'x':
      case 'X': {
        int base;
        int upper = ch == 'X';
        if (ch == 'b')
          base = 2;
        else if (ch == 'u')
          base = 10;
        else if (ch == 'o')
          base = 8;
        else
          base = 16;
        switch (length) {
        case -2: {
          unsigned char val = va_arg(ap, int);
          ret += __print_unsigned(__putchar, val, width, precision, align, pad,
                                  base, upper);
          break;
        }
        case -1: {
          unsigned short val = va_arg(ap, int);
          ret += __print_unsigned(__putchar, val, width, precision, align, pad,
                                  base, upper);
          break;
        }
        case 0: {
          unsigned val = va_arg(ap, unsigned);
          ret += __print_unsigned(__putchar, val, width, precision, align, pad,
                                  base, upper);
          break;
        }
        case 1: {
          unsigned long val = va_arg(ap, unsigned long);
          ret += __print_unsigned(__putchar, val, width, precision, align, pad,
                                  base, upper);
          break;
        }
        case 2: {
          unsigned long val = va_arg(ap, unsigned long);
          ret += __print_unsigned(__putchar, val, width, precision, align, pad,
                                  base, upper);
          break;
        }
        }
      } break;
      case 'f':
        break;
      case 'c': {
        char val = va_arg(ap, int);
        ret += __print_char(__putchar, val, width, align);
      } break;
      case 's': {
        char *val = va_arg(ap, char *);
        ret += __print_str(__putchar, val, width, precision, align);
      } break;
      case 'p': {
        void *val = va_arg(ap, void *);
        ret += __print_unsigned(__putchar, (unsigned long long)val, 16, -1, 0,
                                '0', 16, 1);
      } break;
      case '%': {
        ret += __print_char(__putchar, '%', width, align);
      } break;
        // default:
      }
    } else {
      __putchar(ch);
      ret++;
    }
  }
  return ret;
}
