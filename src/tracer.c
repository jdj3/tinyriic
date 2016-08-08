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
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <string.h>

#include "tr_as.h"
#include "parser.h"
#include "prim.h"

extern tr_addr g_empty;
extern tr_addr g_env;
extern tr_addr g_und;
extern tr_addr g_true;
extern tr_addr g_false;
static pid_t g_pid = -1;

#if 1

#define ARCH_REG_COUNT (26)

char *g_reg_arr[] =
{
    "r15",
    "r14",
    "r13",
    "r12",
    "rbp",
    "rbx",
    "r11",
    "r10",
    "r9",
    "r8",
    "rax",
    "rcx",
    "rdx",
    "rsi",
    "rdi",
    "orig-rax",
    "rip",
    "cs",
    "eflags",
    "rsp",
    "ss",
};

#endif

int parse_fd(int fd)
{
    tr_addr expr_addr;
    tr_addr expr_ret;
    char end;
    int rc;
    
    end = 0;
    rc = parse_expr(fd, &expr_addr, &end);
    
    while (rc == 0)
    {
        expr_ret = prim_eval(expr_addr);
        prim_print(expr_ret);
        rc = parse_expr(fd, &expr_addr, &end);
    }
    
    return rc;
}

tr_addr attach(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_val *pid_val;
    int status;
    long rc;

    if (argc != 2)
    {
        EXCEPTION(ERR_ARG);
        return 0;
    }

    pid_val = lookup_addr_type(argv[0], TR_WORD);
    
    rc = ptrace(PTRACE_ATTACH, pid_val->word, NULL, NULL);

    DBV("PTRACE_ATTACH ret %ld\n", rc);

    if (rc != 0)
    {
        return g_false;
    }
    
    g_pid = pid_val->word;

    rc = waitpid(g_pid, &status, 0);
    
    DBV("waitpid ret %ld 0x%08x\n", rc, status);
    
    prim_eval(alloc_pair(argv[1], g_empty));

    rc = ptrace(PTRACE_DETACH, g_pid, NULL, NULL);
    
    return g_true;
}

tr_addr peek_data(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_val *val;
    long rc;

    if (argc != 1)
    {
        EXCEPTION(ERR_ARG);
        return 0;
    }

    val = lookup_addr_type(argv[0], TR_WORD);
    
    rc = ptrace(PTRACE_PEEKDATA, g_pid, val->word, NULL);

    return alloc_word(rc);
}

tr_addr poke_data(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_val *addr_val;
    tr_val *data_val;

    if (argc != 2)
    {
        EXCEPTION(ERR_ARG);
        return 0;
    }

    addr_val = lookup_addr_type(argv[0], TR_WORD);
    data_val = lookup_addr_type(argv[1], TR_WORD);
    
    ptrace(PTRACE_POKEDATA, g_pid, addr_val->word, data_val->word);

    return g_und;
}

tr_addr get_reg(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_word regs[ARCH_REG_COUNT];
    tr_val *val;
    long rc;
    
    if (argc != 1)
    {
        EXCEPTION(ERR_ARG);
        return 0;
    }
    
    memset(regs, 0, sizeof(regs));
    
    rc = ptrace(PTRACE_GETREGS, g_pid, NULL, regs);

    val = lookup_addr_type(argv[0], TR_WORD);

    return alloc_word(regs[val->word]);
}

tr_addr set_reg(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_word regs[ARCH_REG_COUNT];
    tr_val *reg_val;
    tr_val *val;
    long rc;
    
    if (argc != 2)
    {
        EXCEPTION(ERR_ARG);
        return 0;
    }
    
    memset(regs, 0, sizeof(regs));
    
    rc = ptrace(PTRACE_GETREGS, g_pid, NULL, regs);

    reg_val = lookup_addr_type(argv[0], TR_WORD);
    val = lookup_addr_type(argv[1], TR_WORD);

    regs[reg_val->word] = val->word;
    
    rc = ptrace(PTRACE_SETREGS, g_pid, NULL, regs);

    return g_und;
}

void init_tracer()
{
    tr_addr expr;
    tr_addr def;
    tr_word i;
    
    add_builtin("attach", attach, 0);
    add_builtin("peek-data", peek_data, 0);
    add_builtin("poke-data", poke_data, 0);
    add_builtin("get-reg", get_reg, 0);
    add_builtin("set-reg", set_reg, 0);

    def = alloc_sym("define");
    
    for (i = 0; i < sizeof(g_reg_arr) / sizeof(char *); i++)
    {
        expr = alloc_pair(alloc_word(i), g_empty);
        expr = alloc_pair(alloc_sym(g_reg_arr[i]), expr);
        expr = alloc_pair(def, expr);
        prim_eval(expr);
    }
}

int main(int argc, char **argv)
{
    init_as();
    init_prim();
    init_tracer();
    
    parse_fd(STDIN_FILENO);
    
    return 0;
}
