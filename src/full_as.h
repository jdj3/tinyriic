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

#ifndef FULL_AS_H
#define FULL_AS_H

#include <stdint.h>

#include "tr_types.h"

#define TR_FULL (0x8)
//#define TR_ANY_TYPE (0xff)
#define TR_TYPE_BITS    (4)
#define TR_TYPE_BASE    MAKE_BIT_MASK(TR_TYPE_BITS - 1)
#define TR_TYPE_FULL    MAKE_BIT_MASK(TR_TYPE_BITS)
#define TR_TYPE_DIV     (TR_WORD_BITS / TR_TYPE_BITS)

#define TR_BLK_BITS     (12)
#define TR_TBL_BITS     (10)
#define TR_DIR_BITS     (10)

#define TR_BLK_SHIFT    (0)
#define TR_TBL_SHIFT    (TR_BLK_BITS)
#define TR_DIR_SHIFT    (TR_BLK_BITS + TR_TBL_BITS)

#define TR_BLK_MASK     MAKE_BIT_MASK(TR_BLK_BITS)
#define TR_TBL_MASK     MAKE_BIT_MASK(TR_TBL_BITS)
#define TR_DIR_MASK     MAKE_BIT_MASK(TR_DIR_BITS)

#define TR_BLK_OFF(__a) (((__a) >> TR_BLK_SHIFT) & TR_BLK_MASK)
#define TR_TBL_OFF(__a) (((__a) >> TR_TBL_SHIFT) & TR_TBL_MASK)
#define TR_DIR_OFF(__a) (((__a) >> TR_DIR_SHIFT) & TR_DIR_MASK)

#define MAKE_ADDR(__d, __t, __b)                \
    ((((__d) & TR_DIR_MASK) << TR_DIR_SHIFT) |  \
     (((__t) & TR_TBL_MASK) << TR_TBL_SHIFT) |  \
     (((__b) & TR_BLK_MASK) << TR_BLK_SHIFT))

#define TR_BLK_USED_LEN ((1 << TR_BLK_BITS) / TR_WORD_BITS)
#define TR_TBL_TYPE_LEN ((1 << TR_TBL_BITS) / TR_TYPE_DIV)

typedef struct _tr_blk
{
    tr_word used[TR_BLK_USED_LEN];
    union
    {
        tr_word word[0];
        tr_pair pair[0];
        tr_sym sym[0];
    } arr;
} tr_blk;

typedef struct _tr_tbl
{
    tr_blk *blks[1<<TR_TBL_BITS];
    tr_word types[TR_TBL_TYPE_LEN];
} tr_tbl;

typedef struct _tr_as
{
    tr_tbl *tbl_dir[1<<TR_DIR_BITS];
} tr_as;

typedef int (*val_cmp_fn)(tr_blk *blk, tr_addr blk_off, tr_val *val);

#endif
