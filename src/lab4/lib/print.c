#include "print.h"
#include "sbi.h"

void putc(char c)
{
    sbi_putchar(c);
}

void puts(char *s) {
    while (*s)
        sbi_putchar(*s++);
}

void puti(int x) {
    char buf[10];
    int i = 0;
    if (x == 0)
    {
        sbi_putchar('0');
        return;
    }
    if (x < 0)
    {
        sbi_putchar('-');
        x = -x;
    }
    while (x)
    {
        buf[i++] = x % 10 + '0';
        x /= 10;
    }
    while (i)
        sbi_putchar(buf[--i]);
}
