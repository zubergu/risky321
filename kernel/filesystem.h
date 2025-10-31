#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include "types.h"
#include "kvmem.h"


#define MAX_OS_FILES                             100
#define MAX_FS_FILES                              12
#define MAX_FILESIZE                PAGE_SIZE * 1024

#define USTAR_TYPE_FILE         '0'
#define USTAR_TYPE_HARD_LINK    '1'
#define USTAR_TYPE_SYM_LINK     '2'
#define USTAR_TYPE_CHAR_DEVICE  '3'
#define USTAR_TYPE_BLOCK_DEVICE '4'
#define USTAR_TYPE_DIRECTORY    '5'
#define USTAR_TYPE_NAMED_PIPE   '6'


struct tar_header
{
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char type;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char padding[12];
} __attribute__((packed));


struct filesystem_file
{
    bool exists;
    char name[100];
    char data[MAX_FILESIZE];
    size_t size;
};

struct os_file
{
    bool     in_use;
    uint32_t rwpos;
    uint32_t filesystem_file_idx;
};


void filesystem_init(void);
uint32_t filesystem_file_open(char *filepath, uint32_t mode);
uint32_t filesystem_file_read(struct os_file * file_handle, uint8_t *buf, uint32_t size);
uint32_t filesystem_file_close(uint32_t fd);
uint8_t * filesystem_get_filedata(char *filepath);

#endif