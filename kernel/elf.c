#include "elf.h"
#include "kprintf.h"

void elf_dump_header(struct elf_header * ehp)
{
    kprintf("MAGIC: %x\n", *((uint32_t *)&ehp->magic[0]));
    kprintf("bitsize: %d\n", ehp->bitsize);
    kprintf("endianness: %d\n", ehp->endianness);
    kprintf("version: %d\n", ehp->version);
    kprintf("osabi: %d\n", ehp->osabi);
    kprintf("abiversion: %d\n", ehp->abiversion);
    kprintf("type: %d\n", ehp->type);
    kprintf("machine: %d\n", ehp->machine);
    kprintf("version2: %d\n", ehp->version2);
    kprintf("entry: %x\n", ehp->entry);
    kprintf("phoff: %d\n", ehp->phoff);
    kprintf("shoff: %d\n", ehp->shoff);
    kprintf("flags: %d\n", ehp->flags);
    kprintf("ehsize: %d\n", ehp->ehsize);
    kprintf("phentsize: %d\n", ehp->phentsize);
    kprintf("phnum: %d\n", ehp->phnum);
    kprintf("shentsize: %d\n", ehp->shentsize);
    kprintf("shnum: %d\n", ehp->shnum);
    kprintf("shstrndx: %d\n", ehp->shstrndx);
}

void elf_dump_program_header(struct elf_program_header *ephp)
{
    kprintf("type: %d\n", ephp->type);
    kprintf("offset: %d\n", ephp->offset);
    kprintf("vaddr: %x\n", ephp->vaddr);
    kprintf("paddr: %x\n", ephp->paddr);
    kprintf("filesz: %d\n", ephp->filesz);
    kprintf("memsz: %d\n", ephp->memsz);
    kprintf("flags: %d\n", ephp->flags);
    kprintf("align: %d\n", ephp->align);
}