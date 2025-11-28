#ifndef _VIRTIO_H
#define _VIRTIO_H

#include "types.h"
#include "kvmem.h"

#define VIRTIO_BLK_PADDR 0x10001000 // BASE PHYSICAL ADDRESS OF VIRTIO MMIO BLOCK
#define VIRTIO_IRQ 1

#define SECTOR_SIZE        512
#define VIRTIO_DEVICE_BLK    2

/* Virtio MMIO registers, as offset from VIRTIO0 base address at 0x10001000. */
/* All values should be written/read as little endian */
/* R =  READ ONLY  */
/* W =  WRITE ONLY */
/* RW = READ/WRITE */
#define VIRTIO_MMIO_MAGIC_VALUE		    0x000 // R:  Magic value. Should contain Little Endian value 0x74726976 (ascii equivalen of string "virt" but little endian, so expect "triv" :)
#define VIRTIO_MMIO_VERSION		        0x004 // R:  Device version number should be 0x1 for LEGACY interface version.
#define VIRTIO_MMIO_DEVICE_ID		    0x008 // R:  Virtio Subsytem Device ID. 1 is net, 2 is disk. Many more in documentation
#define VIRTIO_MMIO_VENDOR_ID		    0x00C // R:  Virtio Subsystem Vendor ID: 0x554d4551
#define VIRTIO_MMIO_DEVICE_FEATURES	    0x010 // R:  Bitmap representing features this device supports
#define VIRTIO_MMIO_DEVICE_FEATURES_SEL 0x014 // W:  Device features word selection 
#define VIRTIO_MMIO_DRIVER_FEATURES	    0x020 // W:  Bitmap representing features understood and activated by the driver
#define VIRTIO_MMIO_DRIVER_FEATURES_SEL 0x024 // W:  Driver features word selection
#define VIRTIO_MMIO_QUEUE_SEL		    0x030 // W:  Virtual Queue Index. Index of current queue -> writes/reads from registers below will apply to that queue. First queue is at index 0x0
#define VIRTIO_MMIO_QUEUE_NUM_MAX	    0x034 // R:  Max size of queue at index in register QUEUE_SEL, 0x0 if queue not available
#define VIRTIO_MMIO_QUEUE_NUM		    0x038 // W:  Size of current queue
#define VIRTIO_MMIO_QUEUE_ALIGN         0x03C // W:  Queue Align
#define VIRTIO_MMIO_QUEUE_PFN           0x040 // W:  Physical Frame Number of selected queue
#define VIRTIO_MMIO_QUEUE_READY		    0x044 // RW: Virtual Queue Ready bit. Writing 0x1 to this register notifies device that selected queue is ready to use.
#define VIRTIO_MMIO_QUEUE_NOTIFY	    0x050 // W:  Queue notifier. Writing value to this register notifies device there are new buffers to process in a queue.
#define VIRTIO_MMIO_INTERRUPT_STATUS	0x060 // R:  Interrupt Status
#define VIRTIO_MMIO_INTERRUPT_ACK	    0x064 // W:  Interrupt acknowledge. Writing a bitmap of handled interrupt events as defined in INTERRUPT_STATUS.
#define VIRTIO_MMIO_STATUS		        0x070 // RW: Device Status. Reading returns curent device statsu flags. Writing non-zero value sets status flags. Writing 0x0 triggers device reset.
#define VIRTIO_MMIO_QUEUE_DESC_LOW	    0x080 // W:  Virtual Queue's Descriptor area 64-bit physical address for descriptor table, Lower 32 bits
#define VIRTIO_MMIO_QUEUE_DESC_HIGH	    0x084 // W:  Virtual Queue's Descriptor area 64-bit physical address for descriptor table, Upper 32 bits
#define VIRTIO_MMIO_DRIVER_DESC_LOW	    0x090 // W:  Virtual Queue's Driver Area 64-bit long physical address, lower 32 bits
#define VIRTIO_MMIO_DRIVER_DESC_HIGH	0x094 // W:  Virtual Queue's Driver Area 64-bit long physical address, upper 32 bits
#define VIRTIO_MMIO_DEVICE_DESC_LOW	    0x0a0 // W:  Virtual Queue's Device Area 64-bit long physical address, lower 32 bits
#define VIRTIO_MMIO_DEVICE_DESC_HIGH	0x0A4 // W:  Virtual Queue's Device Area 64-bit long physical address, upper 32 bits
#define VIRTIO_MMIO_CONFIG_GENERATION   0x0FC // R:  Configuration atomicity value
#define VIRTIO_MMIO_CONFIG              0x100 // RW: Configuration Space. Beginning of device specific configuration space starts at this offset. Actual size depends on specific device and driver.


/* STATUS register bits */
#define VIRTIO_STATUS_ACKNOWLEDGE   1
#define VIRTIO_STATUS_DRIVER        2
#define VIRTIO_STATUS_DIRVER_OK     3
#define VIRTIO_STATUS_FEATURES_OK   4


/* Device FEATURE bits */
#define VIRTIO_BLK_F_RO               5  // Disk is read-only
#define VIRTIO_BLK_F_SCSI             7  // Supports scsi
#define VIRTIO_BLK_F_CONFIG_WCE      11
#define VIRTIO_BLK_F_MQ              12 // Support more that one vq
#define VIRTIO_F_ANY_LAYOUT          27
#define VIRTIO_RING_F_INDIRECT_DESC  28
#define VIRTIO_RING_F_EVENT_IDX      29


/* VIRTQ */

#define VIRTQ_AVAIL_F_NO_INTERRUPT    1

/* number of virtio descriptors, power of 2 */
#define VIRTQ_ENTRY_NUM     16

/* single descriptor */
struct virtq_desc
{
    uint64_t addr;
    uint32_t len;
    uint16_t flags;
    uint16_t next;
}__attribute__((packed));

#define VIRTQ_DESC_F_NEXT             1 // descriptor chained with another descriptor
#define VIRTQ_DESC_F_WRITE            2 // device writes


/* the entire avail ring */
struct virtq_avail
{
    uint16_t flags;                 // always zero
    uint16_t index;                 // driver will write ring[idx] next
    uint16_t ring[VIRTQ_ENTRY_NUM]; // descriptor numbers of chain heads
}__attribute__((packed));


/* one entry in the USED ring, with which the device */
/* tells the driver about completed requests */
struct virtq_used_elem
{
    uint32_t id;
    uint32_t len;
}__attribute__((packed));


struct virtq_used
{
    uint16_t flags;  // always zero
    uint16_t index;  // device increments when it adds a ring[] entry
    struct virtq_used_elem ring[VIRTQ_ENTRY_NUM];
}__attribute__((packed));



/* type of first descriptor in virtio-blk disk request*/
#define VIRTIO_BLK_T_IN   0
#define VIRTIO_BLK_T_OUT  1

/* format of the set of block descriptors in a disk request */
struct virtio_blk_req
{
    uint32_t type;      // "first" descriptor virtio_blk_t_in or out
    uint32_t reserved;  
    uint64_t sector;
    uint8_t data[512]; // "second" descriptor
    uint8_t status;    // "third" descriptor
}__attribute__((packed));



struct virtio_virtq
{
    struct virtq_desc descs[VIRTQ_ENTRY_NUM];
    struct virtq_avail avail;
    struct virtq_used used __attribute__((aligned(PAGE_SIZE)));
    int queue_index;
    volatile uint16_t *used_index;
    uint16_t last_used_index;
} __attribute__((packed));

void virtio_blk_init(void);
void virtio_rw(void *buf, uint32_t sector, bool is_write);

#endif
