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

#ifndef SYSLIB_H
#define SYSLIB_H

#include "tr_types.h"

#define NULL    ((void *)0)
#define STDIN   (0)
#define STDOUT  (1)

#define PTRACE_PEEKDATA (2)
#define PTRACE_POKEDATA (5)
#define PTRACE_CONT     (7)
#define PTRACE_GETREGS  (12)
#define PTRACE_SETREGS  (13)
#define PTRACE_ATTACH   (16)
#define PTRACE_DETACH   (17)

void exit(int status);

tr_sword read(int fd, void *buf, tr_word count);

tr_sword write(int fd, void *buf, tr_word count);

void *mmap(void *addr, tr_word length, int prot,
           int flags, int fd, tr_sword pgoffset);

int ptrace(int request, int pid, void *addr, void *data);

int wait4(int pid, int *status, int options, void *rusage);

int select(int nfds, tr_word *readfds, tr_word *writefds,
           tr_word *exceptfds, int *timeout);

void *memset(void *s, int c, tr_word n);

void *memcpy(void *dest, void *src, tr_word n);

tr_word strlen(char *s);

char *strcpy(char *dest, char *src);

int strcmp(char *s1, char *s2);

int puts(char *s);

void puthex(tr_word num);

int parsehex(char *s, tr_word *num);

int parsedec(char *s, tr_word *num);

void *tr_alloc(tr_word size);

unsigned int sleep(unsigned int seconds);

#endif
