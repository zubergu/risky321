#ifndef _ELF_H
#define _ELF_H

#include "types.h"



struct elf_header
{
    uint8_t magic[4];    // 0x7F 'E' 'L' 'F'
    uint8_t bitsize;     
    uint8_t endianness;
    uint8_t version;
    uint8_t osabi;
    uint8_t abiversion;
    uint8_t padding[7];
    uint16_t type;
    uint16_t machine;
    uint32_t version2;
    uint32_t entry;       /* memory address of the entry point */
    uint32_t phoff;      // points to program header table
    uint32_t shoff;      // points to section header table
    uint32_t flags;
    uint16_t ehsize;     // size of this header
    uint16_t phentsize;  // size of program header table entry
    uint16_t phnum;      // number of entries in program header table
    uint16_t shentsize;  // size of section header table entry
    uint16_t shnum;      // number of entries in section header table
    uint16_t shstrndx;   // index of section header table entry containing section names
}__attribute__((packed));



#define ELF_PT_LOAD   0x00000001

#define ELF_PF_X      0x1
#define ELF_PF_W      0x2
#define ELF_PF_R      0x4

struct elf_program_header
{
    uint32_t type;   // identifies the  type of segment
    uint32_t offset; // offset of segment in the file image
    uint32_t vaddr;  // virtual address of segment in memory
    uint32_t paddr;  // segment's physical address
    uint32_t filesz; // size in bytes of segment in file image
    uint32_t memsz;  // size in bytes of the segment in memory
    uint32_t flags;
    uint32_t align; 
}__attribute__((packed));

struct elf_section_header
{
    uint32_t name; // offset to string in .shstrtab section that represents name of this section
    uint32_t type; // type of this header
    uint32_t flags;
    uint32_t addr;  // virtual address of the section in memory, for sections that are loaded
    uint32_t offset; // offset of the section in file image
    uint32_t size;   // size in bytes of the section
    uint32_t link;   
    uint32_t info;   // extra info about the section
    uint32_t addralign; // required alignment of the section. Must be power of 2.
    uint32_t entsize;   // size in bytes of each entry, if section has fixed-size entries  
}__attribute__((packed));


void elf_dump_header(struct elf_header * ehp);
void elf_dump_program_header(struct elf_program_header *ephp);


#endif
