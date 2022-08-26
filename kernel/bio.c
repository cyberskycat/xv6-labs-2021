// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

// struct {
//   struct spinlock lock;
//   struct buf buf[NBUF];

//   // Linked list of all buffers, through prev/next.
//   // Sorted by how recently the buffer was used.
//   // head.next is most recent, head.prev is least.
//   struct buf head;
// } bcache;

#define HSIZE 31
struct bcache_segment {
  struct spinlock lock;
  struct buf buf[NBUF];
  struct buf head;
};

struct bhash_table{
 struct  bcache_segment bs[HSIZE];
};

uint32 hash(uint32 x) {
    // x = ((x >> 16) ^ x) * 0x45d9f3b;
    // x = ((x >> 16) ^ x) * 0x45d9f3b;
    // x = (x >> 16) ^ x;
    // return x%HSIZE;
    return x*2654435761 % HSIZE;
}

struct bcache_segment* getbucket(struct bhash_table* ht,uint blockno){
   uint h = hash(blockno);
  //  printf("get bucket =%d\n",h);
   return &ht->bs[h];
}

struct bhash_table bht;
void
binit(void)
{
  struct buf *b;

  for (int i =0; i<HSIZE;i++){
    struct bcache_segment* bs = &bht.bs[i];
    initlock(&bs->lock, "bcache");
    printf("bcache lock init pointer =%p index=%d size=%d\n",bs,i,NBUF);
    // Create linked list of buffers
    bs->head.prev = &bs->head;
    bs->head.next = &bs->head;
    for(b = bs->buf; b < bs->buf+NBUF; b++){
      b->next = bs->head.next;
      b->prev = &bs->head;
      // printf("b->next =%p b->prev =%p b=%p \n",b->next,b->prev,b);

      initsleeplock(&b->lock, "buffer");
      bs->head.next->prev = b;
      bs->head.next = b;
    }
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  struct bcache_segment* bs =  getbucket(&bht,blockno);
  // printf("get bs pointer=%p \n",bs);
  acquire(&bs->lock);

  // Is the block already cached?
  for(b = bs->head.next; b != &bs->head; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
    // printf("blockno=%d  dev =%d \n",b->blockno,b->dev);
      b->refcnt++;
      release(&bs->lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  for(b = bs->head.prev; b != &bs->head; b = b->prev){
    if(b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      release(&bs->lock);
      acquiresleep(&b->lock);
      return b;
    }
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);
  struct bcache_segment* bs =  getbucket(&bht,b->blockno);
  acquire(&bs->lock);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bs->head.next;
    b->prev = &bs->head;
    bs->head.next->prev = b;
    bs->head.next = b;
  }
  
  release(&bs->lock);
}

void
bpin(struct buf *b) {
  struct bcache_segment* bs =  getbucket(&bht,b->blockno);
  acquire(&bs->lock);
  b->refcnt++;
  release(&bs->lock);
}

void
bunpin(struct buf *b) {
  struct bcache_segment* bs =  getbucket(&bht,b->blockno);
  acquire(&bs->lock);
  b->refcnt--;
  release(&bs->lock);
}


