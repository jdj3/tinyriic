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

#ifndef MICRO_AS_H
#define MICRO_AS_H

#include <stdint.h>

#include "tr_types.h"

#define TR_ADDR_BITS    (16)
#define TR_TYPE_BITS    (2)
#define TR_TYPE_MASK    MAKE_BIT_MASK(TR_TYPE_BITS)
#define TR_TYPE_DIV     (TR_WORD_BITS / TR_TYPE_BITS)
#define TR_PAIR_WORDS   (sizeof(tr_pair) / sizeof(tr_word))
#define TR_SYM_WORDS    (sizeof(tr_sym) / sizeof(tr_word))
#define TR_BLK_SIZE     (1 << 6)
#define TR_BLK_COUNT    ((1 << (TR_ADDR_BITS)) / TR_BLK_SIZE)

typedef struct _tr_as
{
    tr_word *arr;
    tr_word *types;
    tr_addr next[1<<TR_TYPE_BITS];
    tr_addr max;
} tr_as;

typedef int (*val_cmp_fn)(tr_val *a, tr_val *b);

#endif
