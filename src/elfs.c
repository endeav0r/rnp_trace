#include "elfs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct _elfs * elfs_create (pid_t pid)
{
    struct _elfs * elfs;

    elfs = (struct _elfs *) malloc(sizeof(struct _elfs));

    elfs->elfs = NULL;
    elfs->elfs_last = NULL;

    if (elfs_load(elfs, pid))
        return NULL;

    return elfs;
}


void elfs_destroy (struct _elfs * elfs)
{
    struct _elfs_i * next;

    while (elfs->elfs != NULL) {
        next = elfs->elfs->next;
        elfs_i_destroy(elfs->elfs);
        elfs->elfs = next;
    }

    free(elfs);
}


int elfs_load (struct _elfs * elfs, pid_t pid)
{
    char maps_filename[1024];
    FILE * fh;

    snprintf(maps_filename, 1024, "/proc/%d/maps", (int) pid);

    fh = fopen(maps_filename, "r");
    if (fh == NULL)
        return -1;

    char line[512];
    while (fgets(line, 512, fh) != NULL) {
        char str[128];
        char elf_name[128];
        uint64_t addr_lo;
        uint64_t addr_hi;

        sscanf(line, "%lx-%lx %*s %*s %*s %s %s",
               &addr_lo, &addr_hi, str, elf_name);
        
        // create elfs entry with _elf set to NULL. we'll load the ELFs
        // later
        if (elf_name[0] == '/') {
            elfs_insert(elfs, elfs_i_create(addr_lo, addr_hi, elf_name));
        }
    }

    // now it's time to load those elf files
    struct _elfs_i * next;
    struct _elfs_i * ref;
    for (next = elfs->elfs; next != NULL; next = next->next) {
        if (next->elf == NULL) {
            next->elf = elf_load_addr(next->elf_name, next->addr_lo);
            printf("ELF: %s\n", next->elf_name);
            printf("\tbase_address = %llx\n",
                   (unsigned long long) next->elf->base_address);
            printf("\tload_address = %llx\n",
                   (unsigned long long) next->elf->load_address);
            for (ref = next->next; ref != NULL; ref = ref->next) {
                if (    (strcmp(next->elf_name, ref->elf_name) == 0)
                     && (ref->elf == NULL)) {
                    ref->elf = next->elf;
                    elf_reference(next->elf);
                }
            }
        }
    }


    fclose(fh);

    return 0;
}


int elfs_insert (struct _elfs * elfs, struct _elfs_i * elfs_i)
{
    if (elfs->elfs_last == NULL) {
        elfs->elfs = elfs_i;
        elfs->elfs_last = elfs_i;
        return 0;
    }
    else {
        elfs->elfs_last->next = elfs_i;
        elfs->elfs_last = elfs_i;
        return 0;
    }
}


const char * elfs_name (struct _elfs * elfs, uint64_t addr)
{
    struct _elfs_i * elfs_i;

    for (elfs_i = elfs->elfs; elfs_i != NULL; elfs_i = elfs_i->next) {
        if (    (elfs_i->addr_lo <= addr)
             && (elfs_i->addr_hi >  addr))
            return elfs_i->elf_name;
    }

    return NULL;
}


struct _elfs_i * elfs_i_create (uint64_t addr_lo,
                                uint64_t addr_hi,
                                const char * elf_name)
{
    struct _elfs_i * elfs_i;

    elfs_i = (struct _elfs_i *) malloc(sizeof(struct _elfs_i));

    elfs_i->addr_lo = addr_lo;
    elfs_i->addr_hi   = addr_hi;
    elfs_i->elf_name   = strdup(elf_name);
    elfs_i->elf        = NULL;
    elfs_i->next       = NULL;

    return elfs_i;
}


void elfs_i_destroy (struct _elfs_i * elfs_i)
{
    free(elfs_i->elf_name);
    if (elfs_i->elf != NULL)
        elf_destroy(elfs_i->elf);
    free(elfs_i);
}


const char * elfs_symbol (struct _elfs * elfs, uint64_t addr)
{
    struct _elfs_i * elfs_i;
    struct _elfs_i * elfs_i_next;

    elfs_i = elfs->elfs;
    while (elfs_i != NULL) {
        const char * symbol = elf_symbol(elfs_i->elf, addr);
        if (symbol != NULL)
            return symbol;

        elfs_i_next = elfs_i->next;
        while (elfs_i_next != NULL) {
            if (elfs_i->elf == elfs_i_next->elf)
                elfs_i_next = elfs_i_next->next;
            else
                break;
        }
        elfs_i = elfs_i_next;
    }

    return NULL;
}