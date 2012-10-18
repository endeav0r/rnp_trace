#ifndef elfs_HEADER
#define elfs_HEADER

#include <sys/types.h>

#include "elf.h"

struct _elfs_i {
    int flags;
    uint64_t addr_lo;
    uint64_t addr_hi;
    char * elf_name;
    struct _elf * elf;
    struct _elfs_i * next;
};

struct _elfs {
    struct _elfs_i * elfs;
    struct _elfs_i * elfs_last;
};

struct _elfs * elfs_create  (pid_t pid);
void           elfs_destroy (struct _elfs *);
int            elfs_load    (struct _elfs *, pid_t pid);
int            elfs_insert  (struct _elfs *, struct _elfs_i *);
const char *   elfs_name    (struct _elfs *, uint64_t addr);

struct _elfs_i * elfs_i_create  (uint64_t addr_lo,
                                 uint64_t addr_hi,
                                 const char * elf_name);
void             elfs_i_destroy (struct _elfs_i * elfs_i);

const char * elfs_symbol (struct _elfs *, uint64_t addr);

#endif