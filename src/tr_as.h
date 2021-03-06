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

#ifndef TR_AS_H
#define TR_AS_H

#include "tr_types.h"

#ifdef TR_DEBUG
#define EXCEPTION(__e)                                \
    DBSTR("exception ");                              \
    DBHEX((__e));                                     \
    DBSTR(" @ ");                                     \
    DBSTR(__FILE__);                                  \
    DBSTR(":");                                       \
    DBHEX(__LINE__);                                  \
    DBSTR("\n");                                      \
    exception((__e))
#else
#define EXCEPTION exception
#endif

void exception(int err);

tr_val *lookup_addr(tr_addr addr, tr_type *type_ptr);

tr_addr alloc_addr(tr_type type, tr_val *val);

void free_addr(tr_addr addr);

void add_ref(tr_addr addr, int diff);

void init_as();

tr_val *lookup_addr_type(tr_addr addr, tr_type type);

tr_addr alloc_word(tr_word num);

tr_addr alloc_pair(tr_addr car, tr_addr cdr);

tr_addr alloc_sym(char *str);

#endif
