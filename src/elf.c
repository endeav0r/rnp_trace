#include "elf.h"

#include <stdio.h>
#include <stdlib.h>

struct _elf * elf_load (const char * filename)
{
	FILE * fh;
	size_t filesize, bytes_read;
	struct _elf * elf;

	fh = fopen(filename, "rb");
	if (fh == NULL)
		return NULL;

	fseek(fh, 0, SEEK_END);
	filesize = ftell(fh);
	fseek(fh, 0, SEEK_SET);

	elf = (struct _elf *) malloc(sizeof(struct _elf));

	elf->data = (uint8_t *) malloc(filesize);

	bytes_read = fread(elf->data, 1, filesize, fh);

	if (bytes_read != filesize)
		return NULL;

	elf->ehdr32 = (Elf32_Ehdr *) elf->data;
	elf->result = NULL;
	elf->references = 1;
	elf->load_address = 0;

	elf_set_base(elf);

	elf->sym_tree = rat_create((int (*) (void *, void *)) elf_sym_cmp);
	elf_make_symtree(elf);

	return elf;
}


struct _elf * elf_load_addr (const char * filename, uint64_t load_address)
{
	struct _elf * elf;
	elf = elf_load(filename);
	elf->load_address = load_address;
	return elf;
}


void elf_destroy (struct _elf * elf)
{
	if (--(elf->references))
		return;

	free(elf->data);
	if (elf->result != NULL)
		free(elf->result);
	free(elf);
}


void elf_reference (struct _elf * elf)
{
	elf->references++;
}


void elf_set_base (struct _elf * elf)
{
	int phdr_i;
	Elf64_Phdr * phdr64;
	elf->base_address = (uint64_t) -1;

	for (phdr_i = 0; phdr_i < elf->ehdr64->e_phnum; phdr_i++) {
		phdr64 = (Elf64_Phdr *) &(elf->data[elf->ehdr64->e_phoff
			                                + (elf->ehdr64->e_phentsize * phdr_i)]);
		if (    ((uint64_t) phdr64->p_vaddr < (uint64_t) elf->base_address)
			 && (phdr64->p_type == PT_LOAD))
			elf->base_address = (uint64_t) phdr64->p_vaddr;
	}
}


uint64_t elf_entry (struct _elf * elf)
{
	switch (elf->ehdr32->e_ident[EI_CLASS]) {
	case ELFCLASS32 :
		return elf->ehdr32->e_entry;
	case ELFCLASS64 :
		return elf->ehdr64->e_entry;
	}
	return 0;
}


const char * elf_strtab_str (struct _elf * elf, int strtab, int offset)
{
	Elf64_Shdr * shdr64;
	shdr64 = (Elf64_Shdr *) &(elf->data[elf->ehdr64->e_shoff
		                                + (elf->ehdr64->e_shentsize * strtab)]);
	return (const char *) &(elf->data[shdr64->sh_offset + offset]);
}


void elf_make_symtree (struct _elf * elf)
{
	//if (elf->ehdr32->e_ident[EI_CLASS] == ELFCLASS32)
	//	elf32_make_symtree(elf);
	//else if (elf->ehdr32->e_ident[EI_CLASS] = ELFCLASS64)
		elf64_make_symtree(elf);
}


void elf64_make_symtree (struct _elf * elf)
{
	Elf64_Shdr * shdr;
	int          shdr_i = 0;
	Elf64_Sym  * sym;
	int          sym_i  = 0;
	struct _elf_sym elf_sym;

	for (shdr_i = 0; shdr_i < elf->ehdr64->e_shnum; shdr_i++) {
		shdr = (Elf64_Shdr *) &(elf->data[elf->ehdr64->e_shoff
									      + (elf->ehdr64->e_shentsize * shdr_i)]);
		if (shdr->sh_type == SHT_SYMTAB) {
			for (sym_i = 0; sym_i < shdr->sh_size / shdr->sh_entsize; sym_i++) {
				sym = (Elf64_Sym *)
					  &(elf->data[shdr->sh_offset + (shdr->sh_entsize * sym_i)]);
				elf_sym.address = sym->st_value;
				strncpy(elf_sym.name,
						elf_strtab_str(elf, shdr->sh_link, sym->st_name),
						64);
				elf_sym.name[63] = 0;
				if (strlen(elf_sym.name))
					rat_insert(elf->sym_tree, &elf_sym, sizeof(elf_sym));
			}
		}
	}
}


int elf_sym_cmp (struct _elf_sym * lhs, struct _elf_sym * rhs)
{
	if (lhs->address < rhs->address)
		return -1;
	else if (lhs->address > rhs->address)
		return 1;
	return 0;
}


const char * elf_symbol (struct _elf * elf, uint64_t address)
{
	struct _elf_sym elf_sym;
	struct _elf_sym * elf_sym_ptr;

	elf_sym.address = address + (elf->base_address - elf->load_address);
	elf_sym_ptr = rat_search(elf->sym_tree, &elf_sym);

	if (elf_sym_ptr == NULL)
		return NULL;

	return elf_sym_ptr->name;
}