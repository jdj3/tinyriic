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
#include "tr_as.h"
#include "parser.h"
#include "prim.h"

tr_addr g_empty;
tr_addr g_env;
tr_addr g_und;
tr_addr g_func;
tr_addr g_true;
tr_addr g_false;

tr_addr g_quote;
tr_addr g_begin;
tr_addr g_define;
tr_addr g_set_e;
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
            puthex(val->word);
        }
        else if (type == TR_SYM)
        {
            puts(val->sym.str);
        }

        puts(" ");
    }

    puts("\n");

    return g_und;
}

tr_addr add_binding(tr_addr sym, tr_addr val, tr_addr env)
{
    tr_val *env_head;
    tr_addr new_ent;
    tr_addr old_ent;
    
    new_ent = alloc_pair(val, g_empty);
    val = new_ent;
    new_ent = alloc_pair(sym, val);
    free_addr(val);

    env_head = lookup_addr_type(env, TR_PAIR);
    old_ent = env_head->pair.car;
    env_head->pair.car = alloc_pair(new_ent, old_ent);
    free_addr(new_ent);
    free_addr(old_ent);

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

#if 0

void dump_env(tr_addr env)
{
    tr_addr frame;
    tr_type type;
    tr_addr next;
    tr_val *val;
    int rc;

    printf("dump_env %08x\n", env);
    while (env != g_empty)
    {
        val = lookup_addr_type(env, TR_PAIR);
        frame = val->pair.car;
        env = val->pair.cdr;

        printf("frame %08x\n", frame);

        while (frame != g_empty)
        {
            val = lookup_addr_type(frame, TR_PAIR);
            next = val->pair.car;
            frame = val->pair.cdr;

            val = lookup_addr_type(next, TR_PAIR);
            next = val->pair.cdr;
            val = lookup_addr_type(val->pair.car, TR_SYM);
            printf("  sym %-15s ", val->sym.str);
            val = lookup_addr_type(next, TR_PAIR);
            printf("addr %08x ", val->pair.car);
            val = lookup_addr(val->pair.car, &type);
            if (type == TR_WORD)
            {
                printf("word %016lx\n", val->word);
            }
            else if (type == TR_SYM)
            {
                printf("sym  %s\n", val->sym.str);
            }
            else if (type == TR_PAIR)
            {
                printf("pair %08x %08x\n", val->pair.car, val->pair.cdr);
            }
            else
            {
                printf("und\n");
            }
        }
    }
}

#define DBENV dump_env

#else

#define DBENV(...)

#endif

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

    parent_env = env;

tail_expr:

    val = lookup_addr(expr, &type);

    if (type == TR_WORD)
    {
        ret = expr;
        add_ref(ret, 1);
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
        add_ref(ret, 1);
        goto cleanup;
    }

    proc = val->pair.car;
    args = val->pair.cdr;

    if (0)
    {
    }
    else if (proc == g_quote)
    {
        val = lookup_addr_type(args, TR_PAIR);
        ret = val->pair.car;
        add_ref(ret, 1);
    }
    else if (proc == g_begin)
    {
    begin:

        if (args == g_empty)
        {
            ret = g_und;
            goto cleanup;
        }

        val = lookup_addr_type(args, TR_PAIR);

        while (val->pair.cdr != g_empty)
        {
            ret = eval_expr(val->pair.car, env);
            free_addr(ret);
            val = lookup_addr_type(val->pair.cdr, TR_PAIR);
        }

        expr = val->pair.car;
        goto tail_expr;
    }
    else if (proc == g_define)
    {
        val = lookup_addr_type(args, TR_PAIR);
        sym = val->pair.car;
        val = lookup_addr_type(val->pair.cdr, TR_PAIR);
        expr = eval_expr(val->pair.car, env);
        val = lookup_addr_type(env, TR_PAIR);
        next = val->pair.car;

        rc = lookup_frame(sym, next, &var_val);
        if (rc == 0)
        {
            free_addr(var_val->pair.car);
            var_val->pair.car = expr;
            ret = g_und;
        }
        else
        {
            ret = add_binding(sym, expr, env);
            free_addr(expr);
        }
    }
    else if (proc == g_set_e)
    {
        val = lookup_addr_type(args, TR_PAIR);
        sym = val->pair.car;
        val = lookup_addr_type(val->pair.cdr, TR_PAIR);
        expr = eval_expr(val->pair.car, env);

        rc = lookup_env(sym, env, &var_val);
        if (rc != 0)
        {
            EXCEPTION(ERR_BIND);
            return 0;
        }

        free_addr(var_val->pair.car);
        var_val->pair.car = expr;
        ret = g_und;
    }
    else if (proc == g_if)
    {
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

        free_addr(ret);
        expr = val->pair.car;
        goto tail_expr;
    }
    else if (proc == g_and)
    {
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
            free_addr(ret);
            val = lookup_addr_type(val->pair.cdr, TR_PAIR);
        }

        expr = val->pair.car;
        goto tail_expr;
    }
    else if (proc == g_or)
    {
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
            free_addr(ret);
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
            free_addr(expr);
        }

        goto begin;
    }
    else if (proc == g_lambda)
    {
        // TODO: free pairs
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
            add_ref(ret, 1);
            goto cleanup;
        }

        val = lookup_addr_type(proc, TR_PAIR);

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
            argc = length_helper(args);

            {
                tr_addr argv[argc];

                for (i = 0; i < argc; i++)
                {
                    val = lookup_addr_type(args, TR_PAIR);
                    argv[i] = eval_expr(val->pair.car, env);
                    args = val->pair.cdr;
                }

                ret = func(argc, argv, env);
                for (i = 0; i < argc; i++)
                {
                    free_addr(argv[i]);
                }
                free_addr(proc);
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
            ret = eval_expr(val->pair.car, eval_env);
            add_binding(var_val->pair.car, ret, env);
            free_addr(ret);
            next = var_val->pair.cdr;
            args = val->pair.cdr;
        }

        if (args != g_empty)
        {
            EXCEPTION(ERR_ARG);
            return 0;
        }

        if (eval_env != parent_env)
        {
            free_addr(eval_env);
        }

        free_addr(proc);
        args = expr;
        goto begin;
    }

cleanup:

    if (parent_env != env)
    {
        free_addr(env);
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

tr_addr equal(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_word first;
    tr_val *val;
    tr_word i;

    if (argc == 0)
    {
        return g_true;
    }

    val = lookup_addr_type(argv[0], TR_WORD);
    first = val->word;

    for (i = 1; i < argc; i++)
    {
        val = lookup_addr_type(argv[i], TR_WORD);

        if (!(first == val->word))
        {
            return g_false;
        }
    }

    return g_true;
}

tr_addr less_than(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_sword last;
    tr_val *val;
    tr_word i;

    if (argc == 0)
    {
        return g_true;
    }

    val = lookup_addr_type(argv[0], TR_WORD);
    last = val->sword;

    for (i = 1; i < argc; i++)
    {
        val = lookup_addr_type(argv[i], TR_WORD);

        if (!(last < val->sword))
        {
            return g_false;
        }

        last = val->sword;
    }

    return g_true;
}

tr_addr greater_than(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_sword last;
    tr_val *val;
    tr_word i;

    if (argc == 0)
    {
        return g_true;
    }

    val = lookup_addr_type(argv[0], TR_WORD);
    last = val->sword;

    for (i = 1; i < argc; i++)
    {
        val = lookup_addr_type(argv[i], TR_WORD);

        if (!(last > val->sword))
        {
            return g_false;
        }

        last = val->sword;
    }

    return g_true;
}

tr_addr less_equal(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_sword last;
    tr_val *val;
    tr_word i;

    if (argc == 0)
    {
        return g_true;
    }

    val = lookup_addr_type(argv[0], TR_WORD);
    last = val->sword;

    for (i = 1; i < argc; i++)
    {
        val = lookup_addr_type(argv[i], TR_WORD);

        if (!(last <= val->sword))
        {
            return g_false;
        }

        last = val->sword;
    }

    return g_true;
}

tr_addr greater_equal(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_sword last;
    tr_val *val;
    tr_word i;

    if (argc == 0)
    {
        return g_true;
    }

    val = lookup_addr_type(argv[0], TR_WORD);
    last = val->sword;

    for (i = 1; i < argc; i++)
    {
        val = lookup_addr_type(argv[i], TR_WORD);

        if (!(last >= val->sword))
        {
            return g_false;
        }

        last = val->sword;
    }

    return g_true;
}

tr_addr add(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_sword ret;
    tr_val *val;
    tr_word i;

    ret = 0;

    for (i = 0; i < argc; i++)
    {
        val = lookup_addr_type(argv[i], TR_WORD);
        ret += val->sword;
    }

    return alloc_word((tr_word)ret);
}

tr_addr subtract(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_sword ret;
    tr_val *val;
    tr_word i;

    if (argc == 0)
    {
        EXCEPTION(ERR_ARG);
        return 0;
    }

    val = lookup_addr_type(argv[0], TR_WORD);

    if (argc == 1)
    {
        return alloc_word(0 - val->sword);
    }

    ret = val->sword;

    for (i = 1; i < argc; i++)
    {
        val = lookup_addr_type(argv[i], TR_WORD);
        ret -= val->sword;
    }

    return alloc_word((tr_word)ret);
}

tr_addr multiply(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_sword ret;
    tr_val *val;
    tr_word i;

    ret = 1;

    for (i = 0; i < argc; i++)
    {
        val = lookup_addr_type(argv[i], TR_WORD);
        ret *= val->sword;
    }

    return alloc_word((tr_word)ret);
}

tr_addr divide(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_sword ret;
    tr_val *val;
    tr_word i;

    if (argc == 0)
    {
        EXCEPTION(ERR_ARG);
        return 0;
    }

    val = lookup_addr_type(argv[0], TR_WORD);

    if (argc == 1)
    {
        return alloc_word(1 / val->sword);
    }

    ret = val->sword;

    for (i = 1; i < argc; i++)
    {
        val = lookup_addr_type(argv[i], TR_WORD);
        ret /= val->sword;
    }

    return alloc_word((tr_word)ret);
}

tr_addr lognot(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_val *val;

    if (argc != 1)
    {
        EXCEPTION(ERR_ARG);
        return 0;
    }

    val = lookup_addr_type(argv[0], TR_WORD);

    return alloc_word(~(val->word));
}

tr_addr logand(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_word ret;
    tr_val *val;
    tr_word i;

    ret = (tr_word)-1;

    for (i = 0; i < argc; i++)
    {
        val = lookup_addr_type(argv[i], TR_WORD);
        ret &= val->sword;
    }

    return alloc_word(ret);
}

tr_addr logior(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_word ret;
    tr_val *val;
    tr_word i;

    ret = 0;

    for (i = 0; i < argc; i++)
    {
        val = lookup_addr_type(argv[i], TR_WORD);
        ret |= val->sword;
    }

    return alloc_word(ret);
}

tr_addr logxor(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_word ret;
    tr_val *val;
    tr_word i;

    ret = 0;

    for (i = 0; i < argc; i++)
    {
        val = lookup_addr_type(argv[i], TR_WORD);
        ret ^= val->sword;
    }

    return alloc_word(ret);
}

tr_addr ash(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_sword shift;
    tr_sword ret;
    tr_val *val;

    if (argc != 2)
    {
        EXCEPTION(ERR_ARG);
        return 0;
    }

    val = lookup_addr_type(argv[1], TR_WORD);
    shift = val->sword;
    val = lookup_addr_type(argv[0], TR_WORD);

    if (shift < 0)
    {
        shift = 0 - shift;
        ret = val->sword >> (shift % TR_WORD_BITS);
    }
    else
    {
        ret = val->sword << (shift % TR_WORD_BITS);
    }

    return alloc_word((tr_word)ret);
}

tr_addr prim_eval(tr_addr expr)
{
    tr_addr ret;

    ret = eval_expr(expr, g_env);
    free_addr(expr);

    return ret;
}

void prim_print(tr_addr expr)
{
    tr_addr argv[1];

    argv[0] = expr;
    display(1, argv, g_env);
    free_addr(expr);
}

void add_builtin(char *sym, prim_fn fn)
{
    tr_addr new_ent;
    tr_addr addr;

    addr = alloc_word((tr_word)fn);
    new_ent = alloc_pair(addr, g_empty);
    free_addr(addr);
    addr = new_ent;
    new_ent = alloc_pair(g_func, addr);
    free_addr(addr);

    add_binding(alloc_sym(sym), new_ent, g_env);
    free_addr(new_ent);
}

struct builtin_func
{
    char *sym;
    prim_fn fn;
};

struct builtin_func builtin_arr[] =
{
    { "cons", cons },
    { "car", car },
    { "cdr", cdr },
    { "set-car!", set_car_e },
    { "set-cdr!", set_cdr_e },
    { "display", display },
    { "eqv?", eqv_q },
    { "=", equal },
    { "<", less_than },
    { ">", greater_than },
    { "<=", less_equal },
    { ">=", greater_equal },
    { "+", add },
    { "-", subtract },
    { "*", multiply },
    { "/", divide },
    { "lognot", lognot },
    { "logand", logand },
    { "logior", logior },
    { "logxor", logxor },
    { "ash", ash },
};

void init_builtins()
{
    int i;

    for (i = 0; i < sizeof(builtin_arr) / sizeof(struct builtin_func); i++)
    {
        add_builtin(builtin_arr[i].sym, builtin_arr[i].fn);
    }
}

void init_prim()
{
    g_empty = alloc_sym("#empty");
    g_und = alloc_sym("#undefined");
    g_true = alloc_sym("#t");
    g_false = alloc_sym("#f");
    g_func = alloc_sym("#function");
    g_quote = alloc_sym("quote");
    g_begin = alloc_sym("begin");
    g_define = alloc_sym("define");
    g_set_e = alloc_sym("set!");
    g_if = alloc_sym("if");
    g_and = alloc_sym("and");
    g_or = alloc_sym("or");
    g_let = alloc_sym("let");
    g_let_s = alloc_sym("let*");
    g_lambda = alloc_sym("lambda");
    g_env = alloc_pair(g_empty, g_empty);
    init_builtins();
}
