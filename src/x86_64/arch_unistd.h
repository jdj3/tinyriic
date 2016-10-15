#ifndef ARCH_UNISTD_H
#define ARCH_UNISTD_H

#define NR_read     (0)
#define NR_write    (1)
#define NR_mmap     (9)
#define NR_select   (23)
#define NR_exit     (60)
#define NR_wait4    (61)
#define NR_ptrace   (101)

#define MMAP_ANONYMOUS (0x20)
#define MMAP_PRIVATE   (0x2)

#endif
