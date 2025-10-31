#include "virtio.h"
#include "system.h" /* for kpanic()*/
#include "kmem.h"
#include "klibc.h" /* memcpy, offsetof macro */
#include "kprintf.h"

struct virtio_virtq *virtq_init(unsigned int index);

struct virtio_virtq *blk_request_vq;

uint32_t blk_req_paddr;
struct virtio_blk_req *blk_req;

volatile uint64_t blk_capacity;

static uint32_t virtio_reg_read32(unsigned int offset)
{
    return *((volatile uint32_t *)(VIRTIO_BLK_PADDR + offset));
}

static uint64_t virtio_reg_read64(unsigned int offset)
{
    return *((volatile uint64_t *) (VIRTIO_BLK_PADDR + offset));
}

static void virtio_reg_write32(unsigned int offset, uint32_t value)
{
    *((volatile uint32_t*)(VIRTIO_BLK_PADDR + offset)) = value;
}


void virtio_blk_init(void)
{
    uint32_t status = 0;

    /* CHECK IF THIS DEVICE IS ACTUALLY VIRTIO-BLK */
    if(virtio_reg_read32(VIRTIO_MMIO_MAGIC_VALUE) != 0x74726976
        || virtio_reg_read32(VIRTIO_MMIO_VERSION) != 1
        || virtio_reg_read32(VIRTIO_MMIO_DEVICE_ID) != VIRTIO_DEVICE_BLK)
    {
        kpanic(__FILE__, __LINE__, "virtio: unrecognized device");
    }


    /* 1. Reset the device */
    virtio_reg_write32(VIRTIO_MMIO_STATUS, 0x0);

    /* 2. Set the ACKNOWLEDGE status bit: guest OS has noticed the device */
    status |= VIRTIO_STATUS_ACKNOWLEDGE;
    virtio_reg_write32(VIRTIO_MMIO_STATUS, status);

    /* 3. Set the Driver status bit */
    status |= VIRTIO_STATUS_DRIVER;
    virtio_reg_write32(VIRTIO_MMIO_STATUS, status);

    /* 4. Negotiate features */

    /* 5. Tell device that feature negotation is complete */
    status |= VIRTIO_STATUS_FEATURES_OK;
    virtio_reg_write32(VIRTIO_MMIO_STATUS, status);

    /* 6. Re-read STATUS to ensure FEATURES_OK bit is still set */
    status = virtio_reg_read32(VIRTIO_MMIO_STATUS);
    if(!(status & VIRTIO_STATUS_FEATURES_OK))
    {
        kpanic(__FILE__, __LINE__, "virtio disk FEATURES_OK unset");
    }

    /* 7. Device specific setup, discovering virtual queues etc.*/
    /* Initialize queue 0 */
    blk_request_vq = virtq_init(0);

    /* 8. Set the DRIVER_OK status bit */
    status |= VIRTIO_STATUS_DIRVER_OK;
    virtio_reg_write32(VIRTIO_MMIO_STATUS, status);

    /* GET the disk capacity */
    blk_capacity = virtio_reg_read64(VIRTIO_MMIO_CONFIG + 0) * SECTOR_SIZE;

    kprintf("virtio blk_capacity=%d\n", (uint32_t)blk_capacity);

    /* allocate memory to store requests to the virtio-blk device */
    blk_req_paddr = (uint32_t)kmem_alloc();
    blk_req = (struct virtio_blk_req *)blk_req_paddr;
}


struct virtio_virtq *virtq_init(unsigned int index)
{
    /* allocate memory for the virtual queue */
    /*TODO: it needs 2 pages so... */
    /* rewrite this to non-legacy mode, where queues have separate registers for addresses */
    /* dirty workaround is to allocate two pages and hope they are next */
    /* to each other in reverse order, that's how allocator allocates but that needs to change */
    uint32_t virtq_paddr = (uint32_t)kmem_alloc();
    virtq_paddr = (uint32_t)kmem_alloc();

    /* cast allocated memory to virtual queue structure */
    struct virtio_virtq *vq = (struct virtio_virtq *) virtq_paddr;
    vq->queue_index = index;
    vq->used_index = (volatile uint16_t *)&vq->used.index;

    /* 1. Select the queue by writing its index into QUEUE_SEL register */
    virtio_reg_write32(VIRTIO_MMIO_QUEUE_SEL, index);

    /* 5. Notify the device about queue size by writing into QUEUE_NUM register */
    virtio_reg_write32(VIRTIO_MMIO_QUEUE_NUM, VIRTQ_ENTRY_NUM);

    /* 6. Notify the device about the used alignment */
    virtio_reg_write32(VIRTIO_MMIO_QUEUE_ALIGN, 0);

    /* 7. Write physical number of first page of the queue */
    virtio_reg_write32(VIRTIO_MMIO_QUEUE_PFN, virtq_paddr);

    return vq;
}


/* Places given descriptor array index of head descriptor for new block request */
/* at free index in avail ring */
void virtq_kick(struct virtio_virtq *vq, int desc_index)
{
    vq->avail.ring[vq->avail.index % VIRTQ_ENTRY_NUM] = desc_index;
    vq->avail.index++;

    __sync_synchronize();

    virtio_reg_write32(VIRTIO_MMIO_QUEUE_NOTIFY, vq->queue_index);
    vq->last_used_index++;
}

/* check if there are requests being processed by the device */
bool virtq_is_busy(struct virtio_virtq *vq)
{
    return vq->last_used_index != *vq->used_index;
}

void virtio_rw(void *buf, uint32_t sector, bool is_write)
{
    if (sector >= blk_capacity / SECTOR_SIZE) {
        kprintf("virtio: tried to read/write sector=%d, but capacity is %d\n",
              sector, blk_capacity / SECTOR_SIZE);
        return;
    }

    // Construct the request according to the virtio-blk specification.
    blk_req->sector = sector;
    blk_req->type = is_write ? VIRTIO_BLK_T_OUT : VIRTIO_BLK_T_IN;
    if (is_write)
        memcpy(blk_req->data, buf, SECTOR_SIZE);

    // Construct the virtqueue descriptors (using 3 descriptors).
    struct virtio_virtq *vq = blk_request_vq;
    vq->descs[0].addr = blk_req_paddr;
    vq->descs[0].len = sizeof(uint32_t) * 2 + sizeof(uint64_t);
    vq->descs[0].flags = VIRTQ_DESC_F_NEXT;
    vq->descs[0].next = 1;

    vq->descs[1].addr = blk_req_paddr + offsetof(struct virtio_blk_req, data);
    vq->descs[1].len = SECTOR_SIZE;
    vq->descs[1].flags = VIRTQ_DESC_F_NEXT | (is_write ? 0 : VIRTQ_DESC_F_WRITE);
    vq->descs[1].next = 2;

    vq->descs[2].addr = blk_req_paddr + offsetof(struct virtio_blk_req, status);
    vq->descs[2].len = sizeof(uint8_t);
    vq->descs[2].flags = VIRTQ_DESC_F_WRITE;

    /* put descriptor head index (0) into the avail queue and notify disk device*/
    virtq_kick(blk_request_vq, 0);

    /* wait until device finishes processing */
    while(virtq_is_busy(blk_request_vq))
    {
        ;
    }

    /* read status written by disk device, 0 if successful */
    if(blk_req->status != 0)
    {
        kprintf("virtio: failed to read/write sector=%d, status=%d\n",
            sector, blk_req->status);
        return;
    }
    
    /* for read operation, copy data from request into buffer */
    if(!is_write)
    {
        memcpy(buf, blk_req->data, SECTOR_SIZE);
    }
}