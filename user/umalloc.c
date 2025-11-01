#include "umalloc.h"
#include "usyscall.h"
#include "uprintf.h"

struct malloc_header head;
/* 
   head.next ----> will be head_ptr 
   head.prev ----> will be tail_ptr

   ignore size and free for now
*/

void dump_malloc_list();

void * umalloc(uint32_t bytes)
{               
    /* chunk of free memory must fit requested number of bytes + header for remainder */
    /* and be aligned to multiple of 4 bytes (32-bit alignment) */
    /* or if equal size to aligned_bytes - then no need for space for new header*/
    if(bytes == 0)
    {
        return NULL;
    }

    uint32_t aligned_bytes = (((bytes - 1)/4) + 1) * 4;
    uint32_t required_bytes = aligned_bytes + sizeof(struct malloc_header) + 4; // data to malloc + additional header + at least 4 bytes of data to not make new empty segment

    while(TRUE)
    {
        dump_malloc_list();
        if(head.next)
        {
            /* if this not a first allocation request */
            /* then iterate over existing chunk list to find free chunk that can be allocated */

            struct malloc_header * curr_ptr = head.next;

            while(curr_ptr)
            {
                if(curr_ptr->free)
                {
                    if(curr_ptr->size == aligned_bytes)
                    {
                        curr_ptr->free = FALSE;
                        return curr_ptr + 1; /* point to first byte AFTER current malloc header, that's where memory for data starts */
                    }
                    else if(curr_ptr->size >= required_bytes)
                    {
                        /* split this chunk into two, one will be decreased in size and used for current allocation, the other will be created in the remaining memory*/
                        curr_ptr->free = FALSE;

                        /* make a new header in remainder of current chunk */
                        struct malloc_header * new_hdr = (struct malloc_header *)((char *)(curr_ptr + 1) + aligned_bytes);

                        new_hdr->free = TRUE;
                        new_hdr->size = curr_ptr->size - aligned_bytes - sizeof(struct malloc_header);
                        new_hdr->prev = curr_ptr;
                        new_hdr->next = curr_ptr->next;

                        curr_ptr->size = aligned_bytes;
                        curr_ptr->next = new_hdr;

                        return curr_ptr + 1;
                    }
                }

                curr_ptr = curr_ptr->next;

            }
        }

        /* if chunk list doesn't exist yet, or iterated over existing chunk list and not found suitable chunk then allocate next page of memory */
        void * new_chunk = umorecore();

        /* if we run out of core memory*/
        if(new_chunk == NULL)
        {
            return NULL;
        }

    }

    return NULL;
}

void ufree(void *chunk)
{
    /* chunk points to data */
    /* by casting to header and -1, we move to where actual header is */
    struct malloc_header * hdr = ((struct malloc_header *) chunk) - 1;
    uprintf("Freeing %p\n", hdr);
    hdr->free = TRUE;

    /* now if previous or next, or both chunks are free we need to merge them into one */
    if(hdr->next && hdr->next->free)
    {
        hdr->size += sizeof(struct malloc_header) + hdr->next->size;
        hdr->next = hdr->next->next;
        if(hdr->next)
        {
            hdr->next->prev = hdr;
        }

        /* should that be new TAIL? */
        if(hdr->next == NULL)
        {
            head.prev = hdr;
        }
    }



    if(hdr->prev && hdr->prev->free)
    {
        hdr->prev->size += sizeof(struct malloc_header) + hdr->size;
        hdr->prev->next = hdr->next;
        if(hdr->next)
        {
            hdr->next->prev = hdr->prev;
        }

        if(hdr->prev->next == NULL)
        {
            head.prev = hdr->prev;
        }
    }



    dump_malloc_list();
}

void * umorecore()
{
    void * new_page = usys_sbrk();

    /* we got new page from kernel, starting at address returned by morecore and size of 4096 */
    /* sbrk always allocates new full page */
    struct malloc_header * new_chunk_header = (struct malloc_header *) new_page;
    new_chunk_header->free = TRUE;
    new_chunk_header->size = 4096 - sizeof(struct malloc_header);
    new_chunk_header->next = NULL;

    if(head.next)
    {
        /* this is a new tail*/
        head.prev->next = new_chunk_header;
        new_chunk_header->prev = head.prev;
        head.prev = new_chunk_header;
    }
    else
    {
        head.next = head.prev = new_chunk_header;
        new_chunk_header->prev = NULL;
    }

    ufree(new_chunk_header+1);

    return new_page;
}

void dump_malloc_list()
{
    uprintf("CURRENT MALLOC LIST:\n");
    
    struct malloc_header * curr_ptr = head.next;

    while(curr_ptr)
    {
        uprintf("SIZE: %d, FREE=%d\n", curr_ptr->size, curr_ptr->free);
        curr_ptr = curr_ptr->next;
    }

    uprintf("CURRENT MALLOC LIST END.\n");
}
