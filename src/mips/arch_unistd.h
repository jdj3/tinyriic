#ifndef ARCH_UNISTD_H
#define ARCH_UNISTD_H

#define NR_exit     (4001)
#define NR_read     (4003)
#define NR_write    (4004)
#define NR_ptrace   (4026)
#define NR_wait4    (4114)
#define NR_mmap     (4210)
#define NR_select   (4142)

#define MMAP_ANONYMOUS (0x800)
#define MMAP_PRIVATE   (0x2)

#endif
