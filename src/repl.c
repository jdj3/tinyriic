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

extern tr_addr g_env;

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

int main(int argc, char **argv)
{
    init_as();
    init_prim();

    parse_fd(STDIN);
    
    return 0;
}
