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

extern tr_addr g_empty;
extern tr_addr g_env;
extern tr_addr g_und;
extern tr_addr g_true;
extern tr_addr g_false;

static int g_pid;
static tr_addr g_hooks;

#if defined(TR_ARCH_x86_64)

#define ARCH_REG_COUNT (26)
#define ARCH_REG_TYPE  long
#define HOOK_INST      (0xcc)
#define HOOK_LEN       (1)
#define POS_IDX        (0x10)
#define TR_X86_FAMILY

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

#elif defined(TR_ARCH_mips)

#define ARCH_REG_COUNT (38)
#define ARCH_REG_TYPE  long long
#define HOOK_INST      (0xcc)
#define HOOK_LEN       (4)
#define POS_IDX        (0x0)

char *g_reg_arr[] =
{
    "unk-0",
    "unk-1",
    "unk-2",
    "unk-3",
    "unk-4",
    "unk-5",
    "r0",
    "r1",
    "r2",
    "r3",
    "r4",
    "r5",
    "r6",
    "r7",
    "r8",
    "r9",
    "r10",
    "r11",
    "r12",
    "r13",
    "r14",
    "r15",
    "r16",
    "r17",
    "r18",
    "r19",
    "r20",
    "r21",
    "r22",
    "r23",
    "r24",
    "r25",
    "r26",
    "r27",
    "r28",
    "r29",
    "r30",
    "r31",
};

#else

#error invalid arch

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

int peek_poke(tr_word addr, tr_byte *read_buf, tr_byte *write_buf, tr_word len)
{
    tr_byte *num_buf;
    tr_word shift;
    tr_word align;
    tr_word copy;
    tr_word num;
    long rc;

    shift = addr & (sizeof(long)-1);
    addr &= (~(sizeof(long)-1));
    num_buf = (tr_byte *)(&num);

    while (len > 0)
    {
        copy = sizeof(long) - shift;
        if (copy > len)
        {
            copy = len;
        }

        rc = ptrace(PTRACE_PEEKDATA, g_pid, (void *)addr, &num);

        if (read_buf != NULL)
        {
            memcpy(read_buf, num_buf + shift, copy);
            read_buf += copy;
        }

        if (write_buf != NULL)
        {
            memcpy(num_buf + shift, write_buf, copy);
            write_buf += copy;
            ptrace(PTRACE_POKEDATA, g_pid, (void *)addr, (void *)num);
        }

        addr += sizeof(long);
        shift = 0;
        len -= copy;
    }

    return 0;
}

void write_inst(tr_word addr, tr_word inst, tr_word *old_inst)
{
#if (HOOK_LEN == 1)
    tr_byte old;
    tr_byte new;
#elif (HOOK_LEN == 4)
    tr_u32 old;
    tr_u32 new;
#else
#error invalid HOOK_LEN
#endif

    new = inst;
    peek_poke(addr, &old, &new, HOOK_LEN);

    if (old_inst != NULL)
    {
        *old_inst = old;
    }
}

void write_hook(tr_addr hook)
{
    tr_word orig_word;
    tr_val *addr_val;
    tr_word shift;
    tr_word align;
    tr_addr addr;
    tr_byte *ptr;
    tr_val *val;
    long orig;

    val = lookup_addr_type(hook, TR_PAIR);
    addr_val = lookup_addr_type(val->pair.car, TR_WORD);

    write_inst(addr_val->word, HOOK_INST, &orig_word);

    val = lookup_addr_type(val->pair.cdr, TR_PAIR);
    val = lookup_addr_type(val->pair.cdr, TR_PAIR);
    val->pair.car = alloc_word(orig_word);
}

#define INST_READ_LEN   (4)

#define X86_VALID       (1<<0)
#define X86_MODRM       (1<<1)
#define X86_REX         (1<<2)

tr_word x86_inst_len(tr_byte *inst_arr)
{
    tr_byte hi_nyb;
    tr_byte lo_nyb;
    tr_byte rex;
    tr_byte len;
    tr_byte mod;
    tr_byte rm;
    int status;

    status = 0;
    len = 1;

    hi_nyb = (inst_arr[0] & 0xf0) >> 4;
    lo_nyb = inst_arr[0] & 0x0f;

    if (hi_nyb == 0x4)
    {
        status |= X86_REX;
        rex = lo_nyb;
        len += 1;
        inst_arr++;
        hi_nyb = (inst_arr[0] & 0xf0) >> 4;
        lo_nyb = inst_arr[0] & 0x0f;
    }

    if ((hi_nyb == 0x5) ||
        ((hi_nyb == 0x9) && ((lo_nyb & 0x8) == 0x0)))
    {
        status |= X86_VALID;
    }
    else if ((((hi_nyb & 0xc) == 0x0) && ((lo_nyb & 0x4) == 0x0)) ||
             ((hi_nyb == 0x8) && ((lo_nyb & 0xc) == 0x8)))
    {
        status |= X86_VALID | X86_MODRM;
    }
    else if ((hi_nyb == 0x8) && ((lo_nyb & 0xc) == 0x4))
    {
        status |= X86_VALID | X86_MODRM;
    }
    else if (hi_nyb == 0xb)
    {
        status |= X86_VALID;

        if ((lo_nyb & 0x8) == 0x0)
        {
            len += 1;
        }
        else if ((status & X86_MODRM) && ((rex & 0x8) == 0x8))
        {
            len += 8;
        }
        else
        {
            len += 4;
        }
    }

    if (status & X86_MODRM)
    {
        len += 1;
        mod = (inst_arr[1] & 0xc) >> 6;
        rm = inst_arr[1] & 0x7;

        if ((mod != 0x3) && (rm == 0x4))
        {
            //SIB
            len += 1;
        }
        if ((mod == 0x0) && (rm == 0x5))
        {
            len += 4;
        }
        if (mod == 0x1)
        {
            len += 1;
        }
        if (mod == 0x2)
        {
            len += 4;
        }
    }

    if (status & X86_VALID)
    {
        return len;
    }

    return 0;
}

tr_byte get_inst_len(tr_word addr)
{
    tr_byte inst_arr[INST_READ_LEN];
    tr_byte len;

#ifdef TR_X86_FAMILY
    peek_poke(addr, inst_arr, NULL, INST_READ_LEN);

    len = x86_inst_len(inst_arr);

    if (len == 0)
    {
        EXCEPTION(ERR_INST);
    }
#else
    len = 4;
#endif

    return len;
}

int handle_break()
{
    tr_word regs[ARCH_REG_COUNT];
    tr_word orig_word;
    tr_word inst_len;
    tr_val *addr_val;
    tr_val *hook_val;
    tr_addr cb_expr;
    tr_addr cb_ret;
    tr_addr addr;
    tr_addr hook;
    tr_word pos;
    tr_val *val;
    int status;
    long orig;
    long rc;

    memset(regs, 0, sizeof(regs));

    ptrace(PTRACE_GETREGS, g_pid, NULL, regs);
    regs[POS_IDX] -= HOOK_LEN;
    pos = regs[POS_IDX];
    hook = g_hooks;

    while (hook != g_empty)
    {
        val = lookup_addr_type(hook, TR_PAIR);
        hook_val = lookup_addr_type(val->pair.car, TR_PAIR);
        addr_val = lookup_addr_type(hook_val->pair.car, TR_WORD);

        if (addr_val->word == pos)
        {
            break;
        }

        hook = val->pair.cdr;
    }

    if (hook == g_empty)
    {
        EXCEPTION(ERR_STATE);
        return -1;
    }

    hook_val = lookup_addr_type(hook_val->pair.cdr, TR_PAIR);
    cb_expr = hook_val->pair.car;

    hook_val = lookup_addr_type(hook_val->pair.cdr, TR_PAIR);
    hook_val = lookup_addr_type(hook_val->pair.car, TR_WORD);

    write_inst(pos, hook_val->word, NULL);
    ptrace(PTRACE_SETREGS, g_pid, NULL, regs);

    cb_ret = prim_eval(alloc_pair(cb_expr, g_empty));

    if (cb_ret == g_false)
    {
        return -1;
    }

    inst_len = get_inst_len(pos);
    write_inst(pos + inst_len, HOOK_INST, &orig_word);
    ptrace(PTRACE_CONT, g_pid, NULL, NULL);

    rc = wait4(g_pid, &status, 0, NULL);
    DBSTR("wait4 ret ");
    DBHEX(rc);
    DBSTR(" ");
    DBHEX(status);
    DBSTR("\n");

    ptrace(PTRACE_GETREGS, g_pid, NULL, regs);
    regs[POS_IDX] -= HOOK_LEN;

    write_inst(pos, HOOK_INST, NULL);
    write_inst(pos + inst_len, orig_word, NULL);
    ptrace(PTRACE_SETREGS, g_pid, NULL, regs);

    return 0;
}

tr_addr attach(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_addr hook;
    tr_val *val;
    int status;
    long rc;

    if (argc != 2)
    {
        EXCEPTION(ERR_ARG);
        return 0;
    }

    val = lookup_addr_type(argv[0], TR_WORD);

    rc = ptrace(PTRACE_ATTACH, val->word, NULL, NULL);

    DBSTR("PTRACE_ATTACH ret ");
    DBHEX(rc);
    DBSTR("\n");

    if (rc != 0)
    {
        return g_false;
    }
    
    g_pid = val->word;

    rc = wait4(g_pid, &status, 0, NULL);
    DBSTR("wait4 ret ");
    DBHEX(rc);
    DBSTR(" ");
    DBHEX(status);
    DBSTR("\n");

    prim_eval(alloc_pair(argv[1], g_empty));

    hook = g_hooks;

    while (hook != g_empty)
    {
        val = lookup_addr_type(hook, TR_PAIR);
        write_hook(val->pair.car);
        hook = val->pair.cdr;
    }

    rc = 0;

    while (rc == 0)
    {
        rc = ptrace(PTRACE_CONT, g_pid, NULL, NULL);
        rc = wait4(g_pid, &status, 0, NULL);
        DBSTR("wait4 ret ");
        DBHEX(rc);
        DBSTR(" ");
        DBHEX(status);
        DBSTR("\n");

        rc = handle_break();
    }

    rc = ptrace(PTRACE_DETACH, g_pid, NULL, NULL);

    g_pid = -1;

    return g_true;
}

tr_addr peek_data(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_word num;
    tr_val *val;
    long rc;

    if (argc != 1)
    {
        EXCEPTION(ERR_ARG);
        return 0;
    }

    val = lookup_addr_type(argv[0], TR_WORD);

    rc = ptrace(PTRACE_PEEKDATA, g_pid, (void *)(val->word), &num);

    return alloc_word(num);
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
    
    ptrace(PTRACE_POKEDATA, g_pid, (void *)(addr_val->word),
           (void *)(data_val->word));

    return g_und;
}

tr_addr get_reg(tr_word argc, tr_addr *argv, tr_addr env)
{
    ARCH_REG_TYPE regs[ARCH_REG_COUNT];
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

    return alloc_word((tr_word)(regs[val->word]));
}

tr_addr set_reg(tr_word argc, tr_addr *argv, tr_addr env)
{
    ARCH_REG_TYPE regs[ARCH_REG_COUNT];
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

    regs[reg_val->word] = (ARCH_REG_TYPE)val->word;
    
    rc = ptrace(PTRACE_SETREGS, g_pid, NULL, regs);

    return g_und;
}

tr_addr add_hook(tr_word argc, tr_addr *argv, tr_addr env)
{
    tr_addr hook;
    tr_val *val;

    if (argc != 2)
    {
        EXCEPTION(ERR_ARG);
        return 0;
    }

    hook = alloc_word(0);
    hook = alloc_pair(hook, g_empty);
    hook = alloc_pair(argv[1], hook);
    hook = alloc_pair(argv[0], hook);

    g_hooks = alloc_pair(hook, g_hooks);

    if (g_pid != -1)
    {
        write_hook(hook);
    }

    return g_und;
}

void init_tracer()
{
    tr_addr expr;
    tr_addr def;
    tr_word i;
    
    add_builtin("attach", attach);
    add_builtin("peek-data", peek_data);
    add_builtin("poke-data", poke_data);
    add_builtin("get-reg", get_reg);
    add_builtin("set-reg", set_reg);
    add_builtin("add-hook", add_hook);

    def = alloc_sym("define");
    
    for (i = 0; i < sizeof(g_reg_arr) / sizeof(char *); i++)
    {
        expr = alloc_pair(alloc_word(i), g_empty);
        expr = alloc_pair(alloc_sym(g_reg_arr[i]), expr);
        expr = alloc_pair(def, expr);
        prim_eval(expr);
    }

    g_hooks = g_empty;
    g_pid = -1;
}

int main(int argc, char **argv)
{
    init_as();
    init_prim();
    init_tracer();
    
    parse_fd(STDIN);
    
    return 0;
}
