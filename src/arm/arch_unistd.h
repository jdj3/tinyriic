#ifndef ARCH_UNISTD_H
#define ARCH_UNISTD_H

#define NR_exit     (1)
#define NR_read     (3)
#define NR_write    (4)
#define NR_ptrace   (26)
#define NR_wait4    (114)
#define NR_mmap     (192)

#define MMAP_ANONYMOUS (0x20)
#define MMAP_PRIVATE   (0x2)

#endif
