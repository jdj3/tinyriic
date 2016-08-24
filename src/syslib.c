/*
  Copyright (c) 2016 REDLattice, Inc.
  
  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation files
  (the "Software"), to deal in the Software without restriction,
  including without limitation the rights to use, copy, modify, merge,
  publish, distribute, sublicense, and/or sell copies of the Software,
  and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:
  
  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include "arch_unistd.h"
#include "syslib.h"
#include "tr_types.h"

long syscall(long, ...);

void exit(int status)
{
    syscall(NR_exit, status);
}

tr_sword read(int fd, void *buf, tr_word count)
{
    return syscall(NR_read, fd, buf, count);
}

tr_sword write(int fd, void *buf, tr_word count)
{
    return syscall(NR_write, fd, buf, count);
}

void *mmap(void *addr, tr_word length, int prot,
           int flags, int fd, tr_sword pgoffset)
{
    return (void *)syscall(NR_mmap, addr, length, prot, flags, fd, pgoffset);
}

int ptrace(int request, int pid, void *addr, void *data)
{
    return syscall(NR_ptrace, request, pid, addr, data);
}

int wait4(int pid, int *status, int options, void *rusage)
{
    return syscall(NR_wait4, pid, status, options, rusage);
}

void *memset(void *s, int c, tr_word n)
{
    char *ptr;

    ptr = s;

    while (n > 0)
    {
        *ptr = c;
        ptr++;
        n--;
    }

    return s;
}

void *memcpy(void *dest, void *src, tr_word n)
{
    char *sptr;
    char *dptr;

    sptr = src;
    dptr = dest;

    while (n > 0)
    {
        *dptr = *sptr;
        dptr++;
        sptr++;
        n--;
    }

    return dest;
}

tr_word strlen(char *s)
{
    tr_word ret;

    ret = 0;

    while (*s != 0)
    {
        s++;
        ret++;
    }

    return ret;
}

char *strcpy(char *dest, char *src)
{
    return memcpy(dest, src, strlen(src)+1);
}

int strcmp(char *s1, char *s2)
{
    int diff;

    while (1)
    {
        diff = (*s1) - (*s2);

        if ((diff != 0) || ((*s1) == 0))
        {
            return diff;
        }

        s1++;
        s2++;
    };

    return -1;
}

int puts(char *s)
{
    return write(STDOUT, s, strlen(s));
}

#define HEX_DIGITS  (2 * sizeof(tr_word))

void puthex(tr_word num)
{
    char buf[HEX_DIGITS];
    int i;

    for (i = HEX_DIGITS-1; i >= 0; i--)
    {
        buf[i] = (num & 0x0f) + '0';

        if (buf[i] > '9')
        {
            buf[i] += 'a' - ('9'+1);
        }

        num >>= 4;
    }

    write(STDOUT, buf, HEX_DIGITS);
}

int parsehex(char *s, tr_word *num)
{
    tr_word n;
    int count;
    char c;

    n = 0;
    count = 0;

    while (count <= HEX_DIGITS)
    {
        c = *s;

        if (c == 0)
        {
            break;
        }

        n <<= 4;

        if ((c >= '0') && (c <= '9'))
        {
            n |= c - '0';
        }
        else if ((c >= 'a') && (c <= 'f'))
        {
            n |= (c - 'a') + 0xa;
        }
        else if ((c >= 'A') && (c <= 'F'))
        {
            n |= (c - 'A') + 0xa;
        }
        else
        {
            return -1;
        }

        count++;
        s++;
    }

    if (count > HEX_DIGITS)
    {
        return -1;
    }

    *num = n;

    return 0;
}

#define DEC_DIGITS  ((sizeof(tr_word) / 2) * 5)

int parsedec(char *s, tr_word *num)
{
    tr_word n;
    int count;
    char c;

    n = 0;
    count = 0;

    while (count <= DEC_DIGITS)
    {
        c = *s;

        if (c == 0)
        {
            break;
        }

        if (n > (((tr_word)-1) / 10))
        {
            return -1;
        }

        n *= 10;

        if ((c < '0') || (c > '9'))
        {
            return -1;
        }

        c -= '0';

        if (n > (((tr_word)-1) - c))
        {
            return -1;
        }

        n += c;
        count++;
        s++;
    }

    if (count > DEC_DIGITS)
    {
        return -1;
    }

    *num = n;

    return 0;
}

void *tr_alloc(tr_word size)
{
    void *ret;

    ret = mmap(NULL, size, 0x3, 0x22, -1, 0);

    if ((((tr_sword)ret) < 0) && (((tr_sword)ret) > -0x10000))
    {
        ret = NULL;
    }

    return ret;
}
