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
#include "full_as.h"
#include "tr_as.h"

tr_as *g_as;

void exception(int err)
{
    exit(err);
}

tr_word val_type_len(tr_type type)
{
    tr_word ret;

    ret = 0;
    
    switch (type)
    {
    case TR_WORD:
        ret = sizeof(tr_word);
        break;
    case TR_PAIR:
        ret = sizeof(tr_pair);
        break;
    case TR_SYM:
        ret = sizeof(tr_sym);
        break;
    default:
        EXCEPTION(ERR_STATE);
        break;
    }

    return ret;
}

int cmp_word(tr_blk *blk, tr_addr blk_off, tr_val *val)
{
    if (blk->arr.word[blk_off] == val->word)
    {
        return 0;
    }

    return ERR_NONE;
}

int cmp_sym(tr_blk *blk, tr_addr blk_off, tr_val *val)
{
    if (!strcmp(blk->arr.sym[blk_off].str, val->sym.str))
    {
        return 0;
    }
    
    return ERR_NONE;
}

int cmp_type(tr_type type, tr_blk *blk, tr_addr blk_off, tr_val *val)
{
    int ret;

    ret = ERR_NONE;
    
    switch (type)
    {
    case TR_WORD:
        ret = cmp_word(blk, blk_off, val);
        break;
    case TR_PAIR:
        break;
    case TR_SYM:
        ret = cmp_sym(blk, blk_off, val);
        break;
    default:
        EXCEPTION(ERR_STATE);
        break;
    }

    return ret;
}

int lookup_blk(tr_addr addr, tr_tbl **tbl_ptr, tr_blk **blk_ptr,
               tr_addr *blk_off_ptr, tr_type *type_ptr)
{
    tr_word type_shift;
    tr_word type_word;
    tr_addr tbl_idx;
    tr_type type;
    tr_tbl *tbl;
    tr_blk *blk;

    tbl = g_as->tbl_dir[TR_DIR_OFF(addr)];
    
    if (tbl == NULL)
    {
        return ERR_FAULT;
    }
    
    tbl_idx = TR_TBL_OFF(addr);
    blk = tbl->blks[tbl_idx];
    
    if (blk == NULL)
    {
        return ERR_FAULT;
    }
    
    type_word = tbl->types[tbl_idx / TR_TYPE_DIV];
    type_shift = (tbl_idx % TR_TYPE_DIV) * TR_TYPE_BITS;
    
    type = (tr_type)((type_word >> type_shift) & TR_TYPE_FULL);
    
    if (tbl_ptr != NULL)
    {
        *tbl_ptr = tbl;
    }
    
    if (blk_ptr != NULL)
    {
        *blk_ptr = blk;
    }
    
    if (blk_off_ptr != NULL)
    {
        *blk_off_ptr = TR_BLK_OFF(addr);
    }
    
    if (type_ptr != NULL)
    {
        *type_ptr = type;
    }
    
    return 0;
}

tr_type get_full_type(tr_addr addr)
{
    tr_type type;
    int rc;
    
    rc = lookup_blk(addr, NULL, NULL, NULL, &type);

    if (rc != 0)
    {
        EXCEPTION(rc);
        return 0;
    }
    
    return (type & TR_TYPE_FULL);
}

tr_val *lookup_addr(tr_addr addr, tr_type *type_ptr)
{
    tr_addr blk_off;
    tr_type type;
    tr_blk *blk;
    int rc;
    
    rc = lookup_blk(addr, NULL, &blk, &blk_off, &type);
    
    if (rc != 0)
    {
        EXCEPTION(rc);
        return NULL;
    }
    
    type &= TR_TYPE_BASE;
    
    if (type_ptr != NULL)
    {
        *type_ptr = type;
    }
    
    switch (type)
    {
    case TR_WORD:
        return (tr_val *)(blk->arr.word + blk_off);
        break;
    case TR_SYM:
        return (tr_val *)(blk->arr.sym + blk_off);
        break;
    case TR_PAIR:
        return (tr_val *)(blk->arr.pair + blk_off);
        break;
    default:
        EXCEPTION(ERR_STATE);
        break;
    }
    
    return NULL;
}

void set_full_bit(tr_addr blk_base, tr_type full_bit)
{
    tr_word type_shift;
    tr_word type_word;
    tr_addr tbl_idx;
    tr_word mask;
    tr_tbl *tbl;
    int rc;
    
    tbl = g_as->tbl_dir[TR_DIR_OFF(blk_base)];
    
    if (tbl == NULL)
    {
        EXCEPTION(ERR_STATE);
        return;
    }
    
    tbl_idx = TR_TBL_OFF(blk_base);
    
    type_word = tbl->types[tbl_idx / TR_TYPE_DIV];
    type_shift = (tbl_idx % TR_TYPE_DIV) * TR_TYPE_BITS;
    mask = TR_FULL << type_shift;
    
    if (full_bit)
    {
        type_word |= mask;
    }
    else
    {
        type_word &= ~mask;
    }
    
    tbl->types[tbl_idx / TR_TYPE_DIV] = type_word;
}

void alloc_tbl(tr_addr dir_idx)
{
    tr_tbl *tbl;
    
    tbl = tr_alloc(sizeof(tr_tbl));
    
    if (tbl == NULL)
    {
        EXCEPTION(ERR_MEM);
        return;
    }
    
    memset(tbl, 0, sizeof(tr_tbl));
    
    g_as->tbl_dir[dir_idx] = tbl;
}

void alloc_blk(tr_tbl *tbl, tr_addr tbl_idx, tr_type type)
{
    tr_word type_shift;
    tr_word type_word;
    tr_word arr_len;
    tr_blk *blk;
    
    arr_len = 0;
    
    switch(type)
    {
    case TR_WORD:
        arr_len = sizeof(tr_word);
        break;
    case TR_SYM:
        arr_len = sizeof(tr_sym);
        break;
    case TR_PAIR:
        arr_len = sizeof(tr_pair);
        break;
    default:
        EXCEPTION(ERR_STATE);
        break;
    }
    
    arr_len *= (1 << TR_BLK_BITS);
    arr_len += sizeof(tr_blk);
    
    blk = tr_alloc(arr_len);
    
    if (blk == NULL)
    {
        EXCEPTION(ERR_MEM);
        return;
    }
    
    memset(blk, 0, arr_len);
    
    tbl->blks[tbl_idx] = blk;
    
    type_word = tbl->types[tbl_idx / TR_TYPE_DIV];
    type_shift = (tbl_idx % TR_TYPE_DIV) * TR_TYPE_BITS;
    type_word |= (((tr_word)type) << type_shift);
    tbl->types[tbl_idx / TR_TYPE_DIV] = type_word;
}

tr_addr alloc_in_blk(tr_tbl *tbl, tr_addr blk_base, tr_type type, tr_val *val)
{
    tr_addr used_shift;
    tr_word used_word;
    tr_addr used_idx;
    tr_word val_len;
    tr_addr ret;
    tr_blk *blk;

    blk = tbl->blks[TR_TBL_OFF(blk_base)];
    
    used_idx = 0;
    
    while (used_idx < TR_BLK_USED_LEN)
    {
        if (blk->used[used_idx] != ((tr_word)-1))
        {
            break;
        }
        
        used_idx++;
    }
    
    if (used_idx >= TR_BLK_USED_LEN)
    {
        EXCEPTION(ERR_STATE);
        return 0;
    }
    
    used_shift = 0;
    used_word = blk->used[used_idx];
    
    while (used_shift < TR_WORD_BITS)
    {
        if ((used_word & (1 << used_shift)) == 0)
        {
            used_word |= (1 << used_shift);
            blk->used[used_idx] = used_word;
            ret = (used_idx * TR_WORD_BITS) + used_shift;
            val_len = val_type_len(type);
            memcpy(((char *)&(blk->arr)) + (val_len * ret),
                   val, val_len);
            ret += blk_base;
            break;
        }
        
        used_shift++;
    }
    
    if (used_shift >= TR_WORD_BITS)
    {
        EXCEPTION(ERR_STATE);
        return 0;
    }
    
    while (used_idx < TR_BLK_USED_LEN)
    {
        if (blk->used[used_idx] != ((tr_word)-1))
        {
            return ret;
        }
        
        used_idx++;
    }
    
    set_full_bit(blk_base, TR_FULL);
}

int find_val(tr_val *val, tr_type type, tr_addr *addr_ptr);

tr_addr alloc_addr(tr_type type, tr_val *val)
{
    tr_addr blk_base;
    tr_addr dir_idx;
    tr_addr tbl_idx;
    tr_addr ret;
    tr_tbl *tbl;
    int rc;
    
    if ((type == TR_WORD) || (type == TR_SYM))
    {
        rc = find_val(val, type, &ret);
        
        if (rc == 0)
        {
            return ret;
        }
    }
    
    for (dir_idx = 0; dir_idx < (1 << TR_DIR_BITS); dir_idx++)
    {
        if (g_as->tbl_dir[dir_idx] == NULL)
        {
            alloc_tbl(dir_idx);
        }
        
        tbl = g_as->tbl_dir[dir_idx];
        
        for (tbl_idx = 0; tbl_idx < (1 << TR_TBL_BITS); tbl_idx++)
        {
            if (tbl->blks[tbl_idx] == NULL)
            {
                alloc_blk(tbl, tbl_idx, type);
            }
            
            blk_base = MAKE_ADDR(dir_idx, tbl_idx, 0);

            if (get_full_type(blk_base) == type)
            {
                return alloc_in_blk(tbl, blk_base, type, val);
            }
        }
    }
    
    EXCEPTION(ERR_MEM);
    return 0;
}

void add_ref(tr_addr addr, int diff)
{
}

void free_addr(tr_addr addr)
{
}

int val_in_blk(tr_val *val, tr_blk *blk, tr_type type, tr_addr *blk_off_ptr)
{
    tr_addr used_shift;
    tr_word used_word;
    tr_addr used_idx;
    tr_addr blk_off;
    
    for (used_idx = 0; used_idx < TR_BLK_USED_LEN; used_idx++)
    {
        used_word = blk->used[used_idx];
        
        if (used_word != 0)
        {
            for (used_shift = 0; used_shift < TR_WORD_BITS; used_shift++)
            {
                blk_off = (used_idx * TR_WORD_BITS) + used_shift;
                
                if ((used_word & (1 << used_shift)) &&
                    !cmp_type(type, blk, blk_off, val))
                {
                    *blk_off_ptr = blk_off;
                    return 0;
                }
            }
        }
    }

    return ERR_NONE;
}

int find_val(tr_val *val, tr_type type, tr_addr *addr_ptr)
{
    tr_addr blk_base;
    tr_type blk_type;
    tr_addr blk_num;
    tr_addr blk_off;
    tr_blk *blk;
    int rc;
    
    for (blk_num = 0; blk_num < (1 << (TR_DIR_BITS + TR_TBL_BITS)); blk_num++)
    {
        blk_base = blk_num << TR_BLK_BITS;
        
        rc = lookup_blk(blk_base, NULL, &blk, NULL, &blk_type);
        
        if (rc != 0)
        {
            return ERR_NONE;
        }
        
        if (blk_type == type)
        {
            rc = val_in_blk(val, blk, type, &blk_off);
            
            if (rc == 0)
            {
                *addr_ptr = blk_base + blk_off;
                return 0;
            }
        }
    }
    
    return ERR_NONE;
}

void init_as()
{
    g_as = tr_alloc(sizeof(tr_as));
    
    if (g_as == NULL)
    {
        EXCEPTION(ERR_MEM);
        return;
    }
    
    memset(g_as, 0, sizeof(tr_as));
}
