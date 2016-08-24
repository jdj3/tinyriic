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

#include "syslib.h"
#include "micro_as.h"
#include "tr_as.h"

tr_as *g_as;

tr_word g_type_sizes[] = 
{
    0,
    sizeof(tr_word),
    sizeof(tr_pair),
    sizeof(tr_sym),
};

void exception(int err)
{
    exit(err);
}

int cmp_word(tr_val *a, tr_val *b)
{
    if (a->word == b->word)
    {
        return 0;
    }

    return ERR_NONE;
}

int cmp_sym(tr_val *a, tr_val *b)
{
    if (!strcmp(a->sym.str, b->sym.str))
    {
        return 0;
    }
    
    return ERR_NONE;
}

val_cmp_fn g_cmp_fns[] = 
{
    NULL,
    cmp_word,
    NULL,
    cmp_sym,
};

void setup_blk(tr_type type)
{
    tr_addr blk_idx;
    tr_addr blk_off;
    
    if (g_as->next[TR_FREE] > g_as->max - TR_BLK_SIZE)
    {
        EXCEPTION(ERR_MEM);
        return;
    }
    
    g_as->next[type] = g_as->next[TR_FREE];
    g_as->next[TR_FREE] += TR_BLK_SIZE;

    blk_idx = g_as->next[type] / TR_BLK_SIZE;
    blk_off = blk_idx % TR_TYPE_DIV;
    blk_idx = blk_idx / TR_TYPE_DIV;

    g_as->types[blk_idx] |= ((tr_word)type) << (blk_off * TR_TYPE_BITS);
}

tr_type get_type(tr_addr addr)
{
    tr_addr blk_idx;
    tr_addr blk_off;
    tr_word type;
    
    blk_idx = addr / TR_BLK_SIZE;
    blk_off = blk_idx % TR_TYPE_DIV;
    blk_idx = blk_idx / TR_TYPE_DIV;

    type = g_as->types[blk_idx];
    type >>= blk_off * TR_TYPE_BITS;
    type &= TR_TYPE_MASK;
    
    return type;
}
    
void add_ref(tr_addr addr, int diff)
{
    tr_type type;

    type = get_type(addr);

    if (type != TR_SYM)
    {
        g_as->ref[addr] += diff;
    }
}

int find_val(tr_val *val, tr_type type, tr_addr *addr_ptr)
{
    tr_addr ret;
    
    ret = 0;

    while (ret < g_as->next[type])
    {
        if ((ret % TR_BLK_SIZE == 0) && (get_type(ret) != type))
        {
            ret += TR_BLK_SIZE;
        }
        else if (g_cmp_fns[type](val, ((tr_val *)(g_as->arr + ret))) == 0)
        {
            *addr_ptr = ret;
            return 0;
        }
        else
        {
            ret += g_type_sizes[type] / sizeof(tr_word);
        }
    }

    return ERR_NONE;
}

tr_addr alloc_addr(tr_type type, tr_val *val)
{
    tr_word type_words;
    tr_word type_size;
    tr_addr ret;
    int rc;

    if ((type == TR_FREE) || (type >= (1<<TR_TYPE_BITS)))
    {
        EXCEPTION(ERR_STATE);
        return 0;
    }

    if (type == TR_SYM)
    {
        rc = find_val(val, type, &ret);

        if (rc == 0)
        {
            return ret;
        }
    }

    type_size = g_type_sizes[type];
    type_words = type_size / sizeof(tr_word);

    if (g_as->free[type] == 0)
    {
        ret = g_as->next[type];
        g_as->next[type] += type_words;

        if ((g_as->next[type] % TR_BLK_SIZE) == 0)
        {
            setup_blk(type);
        }
    }
    else
    {
        ret = g_as->free[type];
        g_as->free[type] = g_as->ref[ret];
    }

    memcpy(g_as->arr + ret, val, type_size);
    g_as->ref[ret] = 1;

    if (type == TR_PAIR)
    {
        add_ref(val->pair.car, 1);
        add_ref(val->pair.cdr, 1);
    }

    return ret;
}

void free_helper(tr_addr addr, tr_slist *prev)
{
    tr_slist next;
    tr_type type;
    tr_val *val;

    val = lookup_addr(addr, &type);

    if (type == TR_FREE)
    {
        EXCEPTION(ERR_STATE);
        return;
    }
    else if (type == TR_SYM)
    {
        return;
    }

    next.link = prev;

    while (prev != NULL)
    {
        if (prev->u.addr == addr)
        {
            return;
        }
        prev = prev->link;
    }

    // exception if free

    add_ref(addr, -1);

    if (g_as->ref[addr] != 0)
    {
        return;
    }

    if (type == TR_PAIR)
    {
        next.u.addr = addr;
        free_helper(val->pair.car, &next);
        free_helper(val->pair.cdr, &next);
    }

    g_as->ref[addr] = g_as->free[type];
    g_as->free[type] = addr;
}

void free_addr(tr_addr addr)
{
    free_helper(addr, NULL);
}

tr_val *lookup_addr(tr_addr addr, tr_type *type_ptr)
{
    tr_type type;

    type = get_type(addr);

    if ((type == TR_FREE) || (addr >= g_as->next[type]))
    {
        EXCEPTION(ERR_NONE);
        return NULL;
    }

    *type_ptr = type;
    
    return ((tr_val *)(g_as->arr + addr));
}

void init_as()
{
    tr_word max;
    tr_word len;
    char *ptr;

    max = (1<<16);

    len = sizeof(tr_as);
    len += max * sizeof(tr_word);
    len += max * sizeof(tr_addr);
    len += ((max / TR_BLK_SIZE) / TR_TYPE_DIV) * sizeof(tr_word);

    ptr = tr_alloc(len);

    if (ptr == NULL)
    {
        EXCEPTION(ERR_MEM);
        return;
    }

    memset(ptr, 0, len);

    g_as = (tr_as *)ptr;
    ptr += sizeof(tr_as);
    g_as->arr = (tr_word *)ptr;
    ptr += max * sizeof(tr_word);
    g_as->ref = (tr_addr *)ptr;
    ptr += max * sizeof(tr_addr);
    g_as->types = (tr_word *)ptr;

    g_as->next[TR_FREE] = 0;

    setup_blk(TR_SYM);
    setup_blk(TR_WORD);
    setup_blk(TR_PAIR);
}
