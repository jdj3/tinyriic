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

tr_addr eval_expr(tr_addr expr, tr_addr env);

tr_addr apply(tr_addr body, tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_addr arg_list;
    tr_addr new_env;
    tr_type type;
    prim_fn func;
    tr_addr ret;
    tr_val *val;
    tr_word i;
    
    val = lookup_addr(body, &type);
    
    if (type == TR_WORD)
    {
        func = (prim_fn)val->word;
        return func(argc, argv, env);
    }
    
    if (type != TR_PAIR)
    {
        EXCEPTION(ERR_STATE);
        return 0;
    }
    
    new_env = alloc_pair(g_empty, env);

    val = lookup_addr_type(body, TR_PAIR);
    arg_list = val->pair.car;
    body = val->pair.cdr;

    i = 0;

    while (arg_list != g_empty)
    {
        if (i >= argc)
        {
            EXCEPTION(ERR_ARG);
            return 0;
        }
        
        val = lookup_addr_type(arg_list, TR_PAIR);
        add_binding(val->pair.car, argv[i++], new_env);
        arg_list = val->pair.cdr;
    }
    
    if (i != argc)
    {
        EXCEPTION(ERR_ARG);
        return 0;
    }

    ret = g_und;

    while (body != g_empty)
    {
        val = lookup_addr_type(body, TR_PAIR);
        ret = eval_expr(val->pair.car, new_env);
        body = val->pair.cdr;
    }
    
    return ret;
}

tr_addr eval_expr_helper(tr_word argc, tr_addr expr, tr_addr env)
{
    tr_addr argv[argc];
    tr_addr parent;
    tr_type type;
    tr_addr next;
    tr_addr body;
    tr_addr sym;
    tr_val *val;
    tr_word i;

    DBV("eval, argc %ld, env 0x%08x\n", argc, env);

    if (argc < 1)
    {
        EXCEPTION(ERR_EXPR);
        return 0;
    }
    
    i = 0;
    next = expr;

    while (next != g_empty)
    {
        val = lookup_addr_type(next, TR_PAIR);
        argv[i++] = val->pair.car;
        next = val->pair.cdr;
    }

    argv[0] = eval_expr(argv[0], env);

    val = lookup_addr(argv[0], &type);

    if ((type == TR_SYM) && (val->sym.str[0] == '#'))
    {
        return expr;
    }
    else if (type != TR_PAIR)
    {
        EXCEPTION(ERR_TYPE);
        return 0;
    }
    
    sym = val->pair.car;
    val = lookup_addr_type(val->pair.cdr, TR_PAIR);
    body = val->pair.car;
    val = lookup_addr_type(val->pair.cdr, TR_PAIR);
    parent = val->pair.car;

    if (parent == g_empty)
    {
        parent = env;
    }
    
    if (val->pair.cdr != g_empty)
    {
        EXCEPTION(ERR_STATE);
        return 0;
    }
    
    if (sym == g_func)
    {
        for (i = 1; i < argc; i++)
        {
            argv[i] = eval_expr(argv[i], env);
        }
    }
    
    return apply(body, argc-1, argv+1, parent);
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
    tr_val *found_val;
    tr_word argc;
    tr_type type;
    tr_addr ret;
    tr_val *val;
    int rc;

    val = lookup_addr(expr, &type);

    if (type == TR_WORD)
    {
        return expr;
    }
    if (type == TR_SYM)
    {
        if (val->sym.str[0] == '#')
        {
            return expr;
        }
        
        rc = lookup_env(expr, env, &found_val);

        if (rc != 0)
        {
            EXCEPTION(rc);
            return 0;
        }
        
        return found_val->pair.car;
    }
    
    argc = length_helper(expr);

    return eval_expr_helper(argc, expr, env);
}

tr_addr define(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_val *found_val;
    tr_addr val_addr;
    int rc;
    
    if (argc != 2)
    {
        EXCEPTION(ERR_ARG);
        return 0;
    }

    val_addr = eval_expr(argv[1], env);
    
    rc = lookup_frame(argv[0], env, &found_val);

    if (rc == 0)
    {
        found_val->pair.car = val_addr;
        return g_und;
    }
    else
    {
        return add_binding(argv[0], val_addr, env);
    }
}

tr_addr begin(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_addr ret;
    tr_word i;
    
    ret = g_und;

    for (i = 0; i < argc; i++)
    {
        ret = eval_expr(argv[i], env);
    }

    return ret;
}

tr_addr let_helper(tr_word argc, tr_addr *argv, tr_addr env, tr_word star)
{
    tr_word bind_count;
    tr_addr next_bind;
    tr_addr eval_env;
    tr_addr bind_sym;
    tr_addr val_addr;
    tr_val *pair_val;
    tr_addr new_env;
    
    if (argc < 1)
    {
        EXCEPTION(ERR_ARG);
        return 0;
    }
    
    new_env = alloc_pair(g_empty, env);
    
    if (star == 0)
    {
        eval_env = env;
    }
    else
    {
        eval_env = new_env;
    }

    next_bind = argv[0];

    while (next_bind != g_empty)
    {
        pair_val = lookup_addr_type(next_bind, TR_PAIR);
        next_bind = pair_val->pair.cdr;

        pair_val = lookup_addr_type(pair_val->pair.car, TR_PAIR);
        bind_sym = pair_val->pair.car;
        pair_val = lookup_addr_type(pair_val->pair.cdr, TR_PAIR);

        if (pair_val->pair.cdr != g_empty)
        {
            EXCEPTION(ERR_ARG);
            return 0;
        }

        val_addr = eval_expr(pair_val->pair.car, eval_env);
        add_binding(bind_sym, val_addr, new_env);
    }
    
    return begin(argc-1, argv+1, new_env);
}

tr_addr let(tr_word argc, tr_addr *argv, tr_addr env)
{
    return let_helper(argc, argv, env, 0);
}

tr_addr let_s(tr_word argc, tr_addr *argv, tr_addr env)
{
    return let_helper(argc, argv, env, 1);
}

tr_addr quote(tr_word argc, tr_addr *argv, tr_addr env)
{
    if (argc != 1)
    {
        EXCEPTION(ERR_ARG);
        return 0;
    }

    return argv[0];
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

tr_addr and(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_addr ret;
    tr_word i;
    
    ret = g_true;
    i = 0;
    
    while ((i < argc) && (ret != g_false))
    {
        ret = eval_expr(argv[i], env);
        i++;
    }

    return ret;
}

tr_addr or(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_addr ret;
    tr_word i;
    
    ret = g_false;
    i = 0;
    
    while ((i < argc) && (ret == g_false))
    {
        ret = eval_expr(argv[i], env);
        i++;
    }

    return ret;
}

tr_addr prim_if(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_addr cond;
    tr_addr ret;
    
    if ((argc < 2) || (argc > 3))
    {
        EXCEPTION(ERR_ARG);
        return 0;
    }

    ret = g_und;
    cond = eval_expr(argv[0], env);

    if (cond != g_false)
    {
        ret = eval_expr(argv[1], env);
    }
    else if (argc == 3)
    {
        ret = eval_expr(argv[2], env);
    }

    return ret;
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

tr_addr lambda(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_addr body_list;
    tr_addr func_list;

    //DBV("lambda env 0x%08x\n", env);
    
    if (argc < 1)
    {
        EXCEPTION(ERR_ARG);
        return 0;
    }

    body_list = g_empty;

    while (argc > 0)
    {
        argc--;
        body_list = alloc_pair(argv[argc], body_list);
    }

    func_list = alloc_pair(env, g_empty);
    func_list = alloc_pair(body_list, func_list);
    func_list = alloc_pair(g_func, func_list);

    return func_list;
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
    { "define", define, 1 },
    { "quote", quote, 1 },
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
    { "and", and, 1 },
    { "or", or, 1 },
    { "if", prim_if, 1 },
    { "eqv?", eqv_q, 0 },
    { "let", let, 1 },
    { "let*", let_s, 1 },
    { "begin", begin, 1 },
    { "lambda", lambda, 1 },
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
    g_env = alloc_pair(g_empty, g_empty);
    init_builtins();
}
