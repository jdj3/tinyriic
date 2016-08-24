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

#ifndef TR_TYPES_H
#define TR_TYPES_H

#define TR_DEBUG

#ifdef TR_DEBUG

#define DBSTR puts
#define DBHEX puthex

#else

#define DBSTR(...)
#define DBHEX(...)

#endif

typedef unsigned char tr_type;
typedef unsigned char tr_byte;
typedef unsigned int tr_addr;
typedef unsigned int tr_u32;
typedef long tr_sword;
typedef unsigned long tr_word;

#define ERR_MISC    (-1)
#define ERR_MEM     (-2)
#define ERR_NUM     (-3)
#define ERR_TOK     (-4)
#define ERR_PAREN   (-5)
#define ERR_FAULT   (-6)
#define ERR_STATE   (-7)
#define ERR_ARG     (-8)
#define ERR_NONE    (-9)
#define ERR_RANGE   (-10)
#define ERR_TYPE    (-11)
#define ERR_UND     (-12)
#define ERR_BIND    (-13)
#define ERR_EXPR    (-14)
#define ERR_INST    (-15)

#define TR_MAX_TOK (31)
#define TR_MAX_SYM (15)

typedef struct _tr_pair
{
    tr_addr car;
    tr_addr cdr;
} tr_pair;

typedef struct _tr_sym
{
    tr_byte str[TR_MAX_SYM+1];
} tr_sym;

#define TR_WORD_BITS    (sizeof(tr_word) * 8)

#define MAKE_BIT_MASK(__bits) ((1 << (__bits)) - 1)

#define TR_FREE (0)
#define TR_WORD (1)
#define TR_PAIR (2)
#define TR_SYM  (3)

typedef union _tr_val
{
    tr_sword sword;
    tr_word word;
    tr_pair pair;
    tr_sym sym;
} tr_val;

typedef struct _tr_slist
{
    struct _tr_slist *link;
    union
    {
        tr_val val;
        tr_addr addr;
    } u;
} tr_slist;

#endif
