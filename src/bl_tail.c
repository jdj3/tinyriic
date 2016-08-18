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
#include "prim.h"

tr_addr g_empty;
tr_addr g_env;
tr_addr g_und;
tr_addr g_func;
tr_addr g_spec;
tr_addr g_true;
tr_addr g_false;

tr_addr g_quote;
tr_addr g_begin;
tr_addr g_define;
tr_addr g_if;
tr_addr g_and;
tr_addr g_or;
tr_addr g_let;
tr_addr g_let_s;
tr_addr g_lambda;

tr_addr cons(tr_word argc, tr_addr *argv, tr_addr env)
{
    if (argc != 2)
    {
        EXCEPTION(ERR_ARG);
        return 0;
    }
    
    return alloc_pair(argv[0], argv[1]);
}

tr_addr car(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_val *val;
    
    if (argc != 1)
    {
        EXCEPTION(ERR_ARG);
        return 0;
    }
    
    val = lookup_addr_type(argv[0], TR_PAIR);
    
    return val->pair.car;
}

tr_addr cdr(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_val *val;
    
    if (argc != 1)
    {
        EXCEPTION(ERR_ARG);
        return 0;
    }
    
    val = lookup_addr_type(argv[0], TR_PAIR);
    
    return val->pair.cdr;
}

tr_addr set_car_e(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_val *val;
    
    if (argc != 2)
    {
        EXCEPTION(ERR_ARG);
        return 0;
    }
    
    val = lookup_addr_type(argv[0], TR_PAIR);
    
    val->pair.car = argv[1];

    return g_und;
}

tr_addr set_cdr_e(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_val *val;
    
    if (argc != 2)
    {
        EXCEPTION(ERR_ARG);
        return 0;
    }
    
    val = lookup_addr_type(argv[0], TR_PAIR);
    
    val->pair.cdr = argv[1];

    return g_und;
}

tr_word length_helper(tr_addr head)
{
    tr_val *val;
    tr_word ret;

    ret = 0;
    
    while (head != g_empty)
    {
        val = lookup_addr_type(head, TR_PAIR);
        head = val->pair.cdr;
        ret++;
    }
    
    return ret;
}

tr_addr display(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_type type;
    tr_val *val;
    tr_word i;
    
    for (i = 0; i < argc; i++)
    {
        val = lookup_addr(argv[i], &type);

        if (type == TR_WORD)
        {
            printf("%016lx ", val->word);
        }
        else if (type == TR_SYM)
        {
            printf("%16s ", val->sym.str);
        }
    }

    printf("\n");
    
    return g_und;
}

tr_addr add_binding(tr_addr sym, tr_addr val, tr_addr env)
{
    tr_val *env_head;
    tr_addr new_ent;
    
    new_ent = alloc_pair(val, g_empty);
    new_ent = alloc_pair(sym, new_ent);

    env_head = lookup_addr_type(env, TR_PAIR);
    env_head->pair.car = alloc_pair(new_ent, env_head->pair.car);

    return g_und;
}

int lookup_frame(tr_addr sym, tr_addr frame, tr_val **val_ptr)
{
    tr_val *frame_val;
    tr_val *var_val;
    
    while (frame != g_empty)
    {
        frame_val = lookup_addr_type(frame, TR_PAIR);
        var_val = lookup_addr_type(frame_val->pair.car, TR_PAIR);

        if (var_val->pair.car == sym)
        {
            *val_ptr = lookup_addr_type(var_val->pair.cdr, TR_PAIR);
            return 0;
        }

        frame = frame_val->pair.cdr;
    }

    return ERR_BIND;
}

int lookup_env(tr_addr sym, tr_addr env, tr_val **val_ptr)
{
    tr_val *val;
    int rc;

    while (env != g_empty)
    {
        val = lookup_addr_type(env, TR_PAIR);
        
        rc = lookup_frame(sym, val->pair.car, val_ptr);

        if (rc == 0)
        {
            return 0;
        }

        env = val->pair.cdr;
    }

    return ERR_BIND;
}

tr_addr eval_expr(tr_addr expr, tr_addr env)
{
    tr_addr parent_env;
    tr_addr eval_env;
    tr_val *var_val;
    prim_fn func;
    tr_type type;
    tr_addr proc;
    tr_addr next;
    tr_addr args;
    tr_word argc;
    tr_addr ret;
    tr_addr sym;
    tr_val *val;
    tr_word i;
    int rc;

tail_expr:

    val = lookup_addr(expr, &type);

    if (type == TR_WORD)
    {
        ret = expr;
        goto cleanup;
    }

    if (type == TR_SYM)
    {
        if (val->sym.str[0] == '#')
        {
            ret = expr;
            goto cleanup;
        }
        
        rc = lookup_env(expr, env, &val);

        if (rc != 0)
        {
            EXCEPTION(rc);
            return 0;
        }
        
        ret = val->pair.car;
        goto cleanup;
    }

    proc = val->pair.car;
    args = val->pair.cdr;

    if (0)
    {
    }
    else if (proc == g_quote)
    {
        DBV("quote\n");

        val = lookup_addr_type(args, TR_PAIR);
        ret = val->pair.car;
    }
    else if (proc == g_begin)
    {
        DBV("begin\n");

    begin:

        if (args == g_empty)
        {
            ret = g_und;
            goto cleanup;
        }

        val = lookup_addr_type(args, TR_PAIR);

        while (val->pair.cdr != g_empty)
        {
            eval_expr(val->pair.car, env);
            val = lookup_addr_type(val->pair.cdr, TR_PAIR);
        }

        expr = val->pair.car;
        goto tail_expr;
    }
    else if (proc == g_define)
    {
        DBV("define\n");

        val = lookup_addr_type(args, TR_PAIR);
        sym = val->pair.car;
        val = lookup_addr_type(val->pair.cdr, TR_PAIR);
        ret = eval_expr(val->pair.car, env);

        rc = lookup_frame(sym, env, &var_val);
        if (rc == 0)
        {
            var_val->pair.car = ret;
            ret = g_und;
        }
        else
        {
            ret = add_binding(sym, ret, env);
        }
    }
    else if (proc == g_if)
    {
        DBV("if\n");

        val = lookup_addr_type(args, TR_PAIR);
        ret = eval_expr(val->pair.car, env);
        val = lookup_addr_type(val->pair.cdr, TR_PAIR);

        if (ret == g_false)
        {
            if (val->pair.cdr == g_empty)
            {
                return g_und;
            }

            val = lookup_addr_type(val->pair.cdr, TR_PAIR);
        }

        expr = val->pair.car;
        goto tail_expr;
    }
    else if (proc == g_and)
    {
        DBV("and\n");
        
        if (args == g_empty)
        {
            return g_true;
        }

        val = lookup_addr_type(args, TR_PAIR);

        while (val->pair.cdr != g_empty)
        {
            ret = eval_expr(val->pair.car, env);
            if (ret == g_false)
            {
                return ret;
            }
            val = lookup_addr_type(val->pair.cdr, TR_PAIR);
        }

        expr = val->pair.car;
        goto tail_expr;
    }
    else if (proc == g_or)
    {
        DBV("or\n");
        
        if (args == g_empty)
        {
            return g_false;
        }

        val = lookup_addr_type(args, TR_PAIR);

        while (val->pair.cdr != g_empty)
        {
            ret = eval_expr(val->pair.car, env);
            if (ret != g_false)
            {
                return ret;
            }
            val = lookup_addr_type(val->pair.cdr, TR_PAIR);
        }

        expr = val->pair.car;
        goto tail_expr;
    }
    else if ((proc == g_let) || (proc == g_let_s))
    {
        parent_env = env;
        env = alloc_pair(g_empty, parent_env);

        if (proc == g_let)
        {
            eval_env = parent_env;
        }
        else
        {
            eval_env = env;
        }

        val = lookup_addr_type(args, TR_PAIR);
        next = val->pair.car;
        args = val->pair.cdr;

        while (next != g_empty)
        {
            val = lookup_addr_type(next, TR_PAIR);
            next = val->pair.cdr;

            val = lookup_addr_type(val->pair.car, TR_PAIR);
            sym = val->pair.car;
            val = lookup_addr_type(val->pair.cdr, TR_PAIR);

            if (val->pair.cdr != g_empty)
            {
                EXCEPTION(ERR_ARG);
                return 0;
            }

            expr = eval_expr(val->pair.car, eval_env);
            add_binding(sym, expr, env);
        }
        
        goto begin;
    }
    else if (proc == g_lambda)
    {
        ret = alloc_pair(env, g_empty);
        ret = alloc_pair(args, ret);
        ret = alloc_pair(g_func, ret);
    }
    else
    {
        proc = eval_expr(proc, env);

        if (proc == g_func)
        {
            ret = expr;
            goto cleanup;
        }

        val = lookup_addr(proc, &type);

        if ((type == TR_SYM) && (val->sym.str[0] == '#'))
        {
            ret = expr;
            goto cleanup;
        }
        else if (type != TR_PAIR)
        {
            EXCEPTION(ERR_TYPE);
            return 0;
        }

        sym = val->pair.car;

        if (sym != g_func)
        {
            EXCEPTION(ERR_STATE);
            return 0;
        }

        val = lookup_addr_type(val->pair.cdr, TR_PAIR);
        expr = val->pair.car;
        next = val->pair.cdr;

        val = lookup_addr(expr, &type);

        if (type == TR_WORD)
        {
            func = (prim_fn)val->word;
            argc = 0;
            next = args;

            while (next != g_empty)
            {
                val = lookup_addr_type(next, TR_PAIR);
                argc++;
                next = val->pair.cdr;
            }

            {
                tr_addr argv[argc];

                for (i = 0; i < argc; i++)
                {
                    val = lookup_addr_type(args, TR_PAIR);
                    argv[i] = eval_expr(val->pair.car, env);
                    args = val->pair.cdr;
                }

                ret = func(argc, argv, env);
                goto cleanup;
            }
        }
        else if (type != TR_PAIR)
        {
            EXCEPTION(ERR_STATE);
            return 0;
        }

        val = lookup_addr_type(next, TR_PAIR);
        eval_env = env;
        env = alloc_pair(g_empty, val->pair.car);

        if (val->pair.cdr != g_empty)
        {
            EXCEPTION(ERR_STATE);
            return 0;
        }

        val = lookup_addr_type(expr, TR_PAIR);
        next = val->pair.car;
        expr = val->pair.cdr;

        while (next != g_empty)
        {
            var_val = lookup_addr_type(next, TR_PAIR);
            val = lookup_addr_type(args, TR_PAIR);
            add_binding(var_val->pair.car,
                        eval_expr(val->pair.car, eval_env), env);
            next = var_val->pair.cdr;
            args = val->pair.cdr;
        }

        if (args != g_empty)
        {
            EXCEPTION(ERR_ARG);
            return 0;
        }

        args = expr;
        goto begin;
    }

cleanup:

    return ret;
}

tr_addr add(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_val *val;
    tr_word ret;
    tr_word i;
    
    if (argc == 0)
    {
        EXCEPTION(ERR_ARG);
        return 0;
    }

    ret = 0;

    for (i = 0; i < argc; i++)
    {
        val = lookup_addr_type(argv[i], TR_WORD);
        ret += val->word;
    }
    
    return alloc_word(ret);
}

tr_addr subtract(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_val *a;
    tr_val *b;
    
    if (argc != 2)
    {
        EXCEPTION(ERR_ARG);
        return 0;
    }

    a = lookup_addr_type(argv[0], TR_WORD);
    b = lookup_addr_type(argv[1], TR_WORD);

    return alloc_word(a->word - b->word);
}

tr_addr multiply(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_val *val;
    tr_word ret;
    tr_word i;
    
    if (argc == 0)
    {
        EXCEPTION(ERR_ARG);
        return 0;
    }
    
    ret = 1;
    
    for (i = 0; i < argc; i++)
    {
        val = lookup_addr_type(argv[i], TR_WORD);
        ret *= val->word;
    }
    
    return alloc_word(ret);
}

tr_addr divide(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_val *a;
    tr_val *b;
    
    if (argc != 2)
    {
        EXCEPTION(ERR_ARG);
        return 0;
    }

    a = lookup_addr_type(argv[0], TR_WORD);
    b = lookup_addr_type(argv[1], TR_WORD);

    return alloc_word(a->word / b->word);
}

tr_addr eqv_q(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_type a_type;
    tr_type b_type;
    tr_word i;
    tr_val *a;
    tr_val *b;

    if (argc == 0)
    {
        return g_true;
    }

    a = lookup_addr(argv[0], &a_type);

    for (i = 1; i < argc; i++)
    {
        if (a_type == TR_WORD)
        {
            b = lookup_addr(argv[i], &b_type);

            if ((b_type != TR_WORD) || (b->word != a->word))
            {
                return g_false;
            }
        }
        else if (argv[0] != argv[i])
        {
            return g_false;
        }
    }

    return g_true;
}

tr_addr prim_eval(tr_addr expr)
{
    return eval_expr(expr, g_env);
}

void prim_print(tr_addr expr)
{
    tr_addr argv[1];

    argv[0] = expr;
    display(1, argv, g_env);
}

void add_builtin(char *sym, prim_fn fn, int spec)
{
    tr_addr new_ent;

    new_ent = alloc_pair(g_empty, g_empty);
    new_ent = alloc_pair(alloc_word((tr_word)fn), new_ent);
    if (spec)
    {
        new_ent = alloc_pair(g_spec, new_ent);
    }
    else
    {
        new_ent = alloc_pair(g_func, new_ent);
    }
    
    add_binding(alloc_sym(sym), new_ent, g_env);
}

struct builtin_func
{
    char *sym;
    prim_fn fn;
    int spec;
};

struct builtin_func builtin_arr[] =
{
    { "cons", cons, 0 },
    { "car", car, 0 },
    { "cdr", cdr, 0 },
    { "set-car!", set_car_e, 0 },
    { "set-cdr!", set_cdr_e, 0 },
    { "display", display, 0 },
    { "+", add, 0 },
    { "-", subtract, 0 },
    { "*", multiply, 0 },
    { "/", divide, 0 },
    { "eqv?", eqv_q, 0 },
};

void init_builtins()
{
    int i;

    for (i = 0; i < sizeof(builtin_arr) / sizeof(struct builtin_func); i++)
    {
        add_builtin(builtin_arr[i].sym, builtin_arr[i].fn, builtin_arr[i].spec);
    }
}

void init_prim()
{
    g_empty = alloc_sym("#empty");
    g_und = alloc_sym("#undefined");
    g_true = alloc_sym("#t");
    g_false = alloc_sym("#f");
    g_func = alloc_sym("#function");
    g_spec = alloc_sym("#special");
    g_quote = alloc_sym("quote");
    g_begin = alloc_sym("begin");
    g_define = alloc_sym("define");
    g_if = alloc_sym("if");
    g_and = alloc_sym("and");
    g_or = alloc_sym("or");
    g_let = alloc_sym("let");
    g_let_s = alloc_sym("let*");
    g_lambda = alloc_sym("lambda");
    g_env = alloc_pair(g_empty, g_empty);
    init_builtins();
}
