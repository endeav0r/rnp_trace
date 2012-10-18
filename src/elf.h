#ifndef elf_HEADER
#define elf_HEADER

#include <string.h>

#include <elf.h>

#include "rat.h"

/*
*  These are garbage collected objects. Additional references can be added
*  with elf_reference(). elf_destroy() will handle dereferencing appropriately
*/

#define ELF_SYM_NAME_LEN 64

struct _elf {
	uint64_t  load_address;
	uint64_t  base_address;
	int       references;
	uint8_t * data;
	union {
		Elf32_Ehdr * ehdr32;
		Elf64_Ehdr * ehdr64;
	};
	char *        result;
	struct _rat * sym_tree;
};

struct _elf_sym {
	char name[ELF_SYM_NAME_LEN];
	uint64_t address;
};


struct _elf * elf_load      (const char * filename);
struct _elf * elf_load_addr (const char * filename, uint64_t load_address);
void          elf_destroy   (struct _elf *);
void          elf_reference (struct _elf *);
void          elf_set_base  (struct _elf *);

uint64_t      elf_entry      (struct _elf *);
const char *  elf_strtab_str (struct _elf *, int strtab, int offset);

void          elf_make_symtree   (struct _elf *);
void          elf64_make_symtree (struct _elf *);

int           elf_sym_cmp (struct _elf_sym *, struct _elf_sym *);
const char *  elf_symbol  (struct _elf *, uint64_t address);

#endif