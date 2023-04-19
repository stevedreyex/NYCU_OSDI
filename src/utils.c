#include "mini_uart.h"

/**
 * Helper function to convert ASCII octal number into binary
 * s string
 * n number of digits
 */
int oct2bin(char *s, int n) {
  int r = 0;
  while (n-- > 0) {
    r <<= 3;
    r += *s++ - '0';
  }
  return r;
}

int hex2bin(char *s, int n) {
  int r = 0;
  while (n-- > 0) {
    r <<= 4;
    if (*s <= 'F' && *s >= 'A') {
      r += *s++ - 0x37;
    } else
      r += *s++ - '0';
  }
  return r;
}

int exp(int i, int j) {
  int ret = 1;
  for (; j > 0; j--) {
    ret *= i;
  }
  return ret;
}

void uart_int(int i) {
  int e = 0;
  int temp = i;
  while ((i /= 10) > 0) {
    e++;
  }
  for (; e; e--) {
    uart_send((temp / exp(10, e)) % 10 + '0');
  }
  uart_send('0' + (temp % 10));
}

int atoi(const char* str) {
    int result = 0;
    int sign = 1;
    if (*str == '-') {
        sign = -1;
        str++;
    }
    while (*str != '\0') {
        result = result * 10 + (*str - '0');
        str++;
    }
    return sign * result;
}
// void uart_printf(char *s, )

void buf_clear(char *buf, int BUF_SIZE) {
  for (int i = 0; i < BUF_SIZE; i++) {
    buf[i] = '\0';
  }
}

int memcmp(void *s1, void *s2, int n) {
  unsigned char *a = s1, *b = s2;
  while (n-- > 0) {
    if (*a != *b) {
      return *a - *b;
    }
    a++;
    b++;
  }
  return 0;
}

void delay(int sec) {
  for (int i = 0; i < sec * 1000000000; i++) {
    asm("nop;");
  }
}

void strcpy(char *dest,  char *src)
{
    char *cur = src;
    char *des_cur = dest;
    for(; *cur != '\0'; cur++, des_cur++) {
        *des_cur = *cur;   
    }
    *des_cur = '\0';
}

void strncpy(char *dest,  char *src, int n)
{
    char *cur = src;
    char *des_cur = dest;
    for(; n > 0; cur++, des_cur++, n--) {
        *des_cur = *cur;   
    }
    *des_cur = '\0';
}
