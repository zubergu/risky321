#include "filesystem.h"
#include "kprintf.h"
#include "virtio.h"
#include "klibc.h"
#include "system.h"
#include "kproc.h"


extern uint64_t blk_capacity;

char buffer[SECTOR_SIZE];


/* in-memory view of all files from hard drive, extracted from tar file format */
/* in normal Filesystem, they shouldnt be in memory all the time, that is TODO later */
struct filesystem_file filesystem_files[MAX_FS_FILES];

/* table of all files opened by operating system, may contain same file opened multiple times */
/* each os_file has a cursor/position in file that it will read from/write to */
/* OS_FILE is like a POV into file in filesystem, you can open the same file multiple times, and r/w to different positions in file */
struct os_file         os_files[MAX_OS_FILES];

static int oct2int(char *oct, int len)
{
    int result = 0;

    for(int i = 0; i < len; i++)
    {
        result = (result * 8) + (oct[i] - '0');
    }

    return result;
}


void filesystem_init(void)
{
    uint64_t sector_cnt = blk_capacity / SECTOR_SIZE;
    uint64_t current_sector = 0;

    int i = 0;

    while(i < MAX_FS_FILES && current_sector < sector_cnt )
    {
        virtio_rw(buffer,current_sector, FALSE);
        struct tar_header * th = (struct tar_header *) &buffer[0];

        kprintf("%s\n", th->name);
        kprintf("%s\n", th->size);
        if(th->name[0] == '\0')
        {
            break;
        }

        memcpy(filesystem_files[i].name, th->name, sizeof(th->name));
        uint32_t data_size = oct2int(th->size, sizeof(th->size)-1);
        filesystem_files[i].size = data_size;
        filesystem_files[i].exists = TRUE;


        uint32_t copied_data = 0;
        uint32_t copied_sectors = 0;

        current_sector++;

        while(copied_data < filesystem_files[i].size)
        {
            virtio_rw(&filesystem_files[i].data[copied_sectors * SECTOR_SIZE], current_sector, FALSE);
            current_sector++;
            copied_sectors++;
            copied_data += SECTOR_SIZE;
            if(copied_sectors == (MAX_FILESIZE/SECTOR_SIZE) && copied_data < filesystem_files[i].size)
            {
                kprintf("%s\n", filesystem_files[i].name);
                kpanic(__FILE__, __LINE__, "filesystem: File too big");
            }
        }

        i++;
    }


    for(int i = 0; i < MAX_FS_FILES; i++)
    {
        if(!filesystem_files[i].exists)
        {
            return;
        }
        kprintf("%d: FILE NAME = %s\n", i, filesystem_files[i].name);
        kprintf("    FILE_SIZE = %d\n",    filesystem_files[i].size);
    }
}

/* find file on filesystem that matches given filepath exactly */
uint32_t filesystem_locate_file(char *filepath)
{
    for(uint32_t idx = 0; idx < MAX_FS_FILES; idx++)
    {
        if(strcmp((const char *)filepath, (const char *)filesystem_files[idx].name) == 0)
        {
            return idx;
        }
    }

    return -1;
}

uint8_t * filesystem_get_filedata(char *filepath)
{
    uint32_t idx = filesystem_locate_file(filepath);

    if(idx == 0xFFFFFFFF)
    {
        return NULL;
    }
    else
    {
        return (uint8_t *)filesystem_files[idx].data;
    }
}


uint32_t filesystem_file_open(char *filepath, uint32_t mode)
{
    uint32_t fs_file_idx = filesystem_locate_file(filepath);
    if(fs_file_idx == 0xFFFFFFFF)
    {
        return 0xFFFFFFFF;
    }
    else
    {
        // find space in os_file table, where all files currently opened by OS are located 
        uint32_t os_file_idx = 0;
        for(os_file_idx = 0; os_file_idx < MAX_OS_FILES; os_file_idx++)
        {
            if(!os_files[os_file_idx].in_use)
            {
                break;
            }
        }

        // Couldn't open another file at Operating System level
        if(os_file_idx >= MAX_OS_FILES)
        {
            return -1;
        }

        os_files[os_file_idx].in_use = TRUE;                      /* mark that this structure is already taken */
        os_files[os_file_idx].filesystem_file_idx = fs_file_idx;  /* to locate file in filesystem */
        os_files[os_file_idx].rwpos = 0;                          /* to locate next r/w location inside file data */

        /* add pointer to os_file into current process's opened files array and return FD(index in this array) to user */
        return kproc_file_add(&os_files[os_file_idx]);
    }

    return -1;
}

uint32_t filesystem_file_read(struct os_file * file_handle, uint8_t *buf, uint32_t size)
{
    uint32_t fs_file_idx = file_handle->filesystem_file_idx;
    uint32_t cursor = file_handle->rwpos;
    uint32_t file_size = filesystem_files[fs_file_idx].size;

    int read_bytes = 0;

    while(read_bytes < size && cursor < file_size)
    {
        *buf = filesystem_files[fs_file_idx].data[cursor];
        buf++;
        cursor++;
        read_bytes++;
    }

    file_handle->rwpos = cursor;

    return read_bytes;
}

uint32_t filesystem_file_close(uint32_t fd)
{
    /* ignore closing of stind/stdout/stderr for now */
    if(fd < 3)
    {
        return 0;
    }
    
    struct os_file * file_handle = kproc_file_get(fd);
    /* remove opened file from current process's table */
    if(kproc_file_remove(fd) != 0)
    {
        return -1;
    }

    /* close everything opened on filesystem side as well */
    file_handle->filesystem_file_idx = 0xFFFFFFFF;
    file_handle->rwpos = 0;
    file_handle->in_use = FALSE;

    return 0;
}