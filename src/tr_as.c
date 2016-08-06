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

#include <string.h>
#include <stdio.h>

#include "tr_types.h"
#include "tr_as.h"

tr_val *lookup_addr_type(tr_addr addr, tr_type type)
{
    tr_type ret_type;
    tr_val *ret;

    ret = lookup_addr(addr, &ret_type);

    if (ret_type != type)
    {
        EXCEPTION(ERR_TYPE);
        return NULL;
    }

    return ret;
}

tr_addr alloc_word(tr_word num)
{
    tr_addr ret;
    tr_val val;
    
    val.word = num;
    
    ret = alloc_addr(TR_WORD, &val);
    
    return ret;
}

tr_addr alloc_pair(tr_addr car, tr_addr cdr)
{
    tr_addr ret;
    tr_val val;
    
    val.pair.car = car;
    val.pair.cdr = cdr;
    
    ret = alloc_addr(TR_PAIR, &val);
    
    return ret;
}

tr_addr alloc_sym(char *str)
{
    tr_addr ret;
    tr_val val;
    
    if (strlen(str) > TR_MAX_TOK)
    {
        EXCEPTION(ERR_RANGE);
        return 0;
    }
    
    strcpy(val.sym.str, str);
    
    ret = alloc_addr(TR_SYM, &val);
    
    return ret;
}
