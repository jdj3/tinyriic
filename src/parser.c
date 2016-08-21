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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "tr_as.h"
#include "parser.h"

extern tr_addr g_empty;

int in_tok(char c)
{
    return ((c > ' ') && (c != '(') && (c != ')'));
}

int read_tok(int fd, char *buf, char *end)
{
    int rc;
    int i;
    
    do
    {
        rc = read(fd, buf, 1);
    } while ((rc == 1) && (buf[0] <= ' '));
    
    i = 1;
    
    if (rc != 1)
    {
        goto err;
    }
    
    while ((rc == 1) && (i <= TR_MAX_TOK) && in_tok(buf[i-1]))
    {
        rc = read(fd, buf+i, 1);
        i += 1;
    }
    
    if ((rc == 1) && !in_tok(buf[i-1]))
    {
        *end = buf[i-1];
        buf[i-1] = 0;
        return 0;
    }
    
err:
    buf[i-1] = 0;
    *end = 0;
    return -1;
}

#ifdef TR_DEBUG

char *g_type_strs[] =
{
    "****",
    "word",
    "pair",
    "sym ",
};

void dump_expr(tr_addr expr_addr, int indent, int max_indent)
{
    tr_type type;
    tr_val *car_val;
    tr_val *val;
    int i;

    if (indent > max_indent)
    {
        return;
    }
    
    for (i = 0; i < indent; i++)
    {
        printf("  ");
    }
    
    val = lookup_addr(expr_addr, &type);
    
    printf("%s ", g_type_strs[type]);
    
    switch (type)
    {
    case TR_WORD:
        printf("%p\n", (void *)(val->word));
        break;
    case TR_PAIR:
        printf("0x%08x\n", expr_addr);
        dump_expr(val->pair.car, indent + 1, max_indent);
        dump_expr(val->pair.cdr, indent + 1, max_indent);
        break;
    case TR_SYM:
        printf("%s\n", val->sym.str);
        break;
    default:
        break;
    }
}

#endif

int parse_expr(int fd, tr_addr *addr, char *bound)
{
    char cur_tok[TR_MAX_TOK+1];
    unsigned long num;
    tr_addr old_tail;
    tr_addr new_tail;
    tr_addr sub_addr;
    char *endptr;
    tr_addr ret;
    tr_val *val;
    char end;
    int len;
    int rc;

    if (*bound == '(')
    {
        cur_tok[0] = 0;
        rc = 0;
    }
    else
    {
        rc = read_tok(fd, cur_tok, bound);
    }

    if (rc != 0)
    {
        return rc;
    }

    if ((cur_tok[0] >= '0') && (cur_tok[0] <= '9'))
    {
        num = strtoul(cur_tok, &endptr, 0);
        if (*endptr != 0)
        {
            return ERR_NUM;
        }
        ret = alloc_word((tr_word)num);
    }
    else if (cur_tok[0] != 0)
    {
        ret = alloc_sym(cur_tok);
    }
    else if (*bound == '(')
    {
        old_tail = 0;
        new_tail = 0;
        sub_addr = 0;
        ret = g_empty;

        *bound = 0;

        while (end != ')')
        {
            end = 0;
            sub_addr = g_empty;

            rc = parse_expr(fd, &sub_addr, &end);

            //DBV("sub rc %d, addr 0x%08x, end 0x%02x\n", rc, sub_addr, end);

            // close paren without new token
            if (rc == ERR_PAREN)
            {
                break;
            }
            // other error
            else if (rc != 0)
            {
                return rc;
            }

            new_tail = alloc_pair(sub_addr, g_empty);
            free_addr(sub_addr);

            if (ret == g_empty)
            {
                ret = new_tail;
            }
            else
            {
                val = lookup_addr_type(old_tail, TR_PAIR);
                val->pair.cdr = new_tail;
            }

            old_tail = new_tail;
        }
    }
    else if (*bound == ')')
    {
        return ERR_PAREN;
    }

    *addr = ret;

    return 0;
}
