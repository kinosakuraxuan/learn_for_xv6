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

#define NBUCKET 13

struct {
  // Misses are serialized so that two CPUs cannot install duplicate
  // buffers for the same disk block.
  struct spinlock evictlock;
  struct buf buf[NBUF];
  struct {
    struct spinlock lock;
    struct buf head;
  } bucket[NBUCKET];
} bcache;

static uint
bhash(uint blockno)
{
  return blockno % NBUCKET;
}

void
binit(void)
{
  struct buf *b;

  initlock(&bcache.evictlock, "bcache.evict");
  for(int i = 0; i < NBUCKET; i++){
    initlock(&bcache.bucket[i].lock, "bcache.bucket");
    bcache.bucket[i].head.prev = &bcache.bucket[i].head;
    bcache.bucket[i].head.next = &bcache.bucket[i].head;
  }

  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    uint i = (b - bcache.buf) % NBUCKET;
    struct buf *head = &bcache.bucket[i].head;
    b->next = head->next;
    b->prev = head;
    initsleeplock(&b->lock, "buffer");
    head->next->prev = b;
    head->next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  uint h = bhash(blockno);
  struct buf *head = &bcache.bucket[h].head;

  acquire(&bcache.bucket[h].lock);

  // Is the block already cached?
  for(b = head->next; b != head; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.bucket[h].lock);
      acquiresleep(&b->lock);
      return b;
    }
  }
  release(&bcache.bucket[h].lock);

  acquire(&bcache.evictlock);

  // A different CPU may have inserted this block while we waited.
  acquire(&bcache.bucket[h].lock);
  for(b = head->next; b != head; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.bucket[h].lock);
      release(&bcache.evictlock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Recycle an unused buffer from any bucket. Holding evictlock keeps
  // the uniqueness check and the move into the target bucket atomic
  // with respect to other cache misses.
  for(int i = 0; i < NBUCKET; i++){
    struct buf *oldhead = &bcache.bucket[i].head;
    if(i != h)
      acquire(&bcache.bucket[i].lock);

    for(b = oldhead->prev; b != oldhead; b = b->prev){
      if(b->refcnt == 0){
        b->prev->next = b->next;
        b->next->prev = b->prev;
        if(i != h)
          release(&bcache.bucket[i].lock);

        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;
        b->next = head->next;
        b->prev = head;
        head->next->prev = b;
        head->next = b;

        release(&bcache.bucket[h].lock);
        release(&bcache.evictlock);
        acquiresleep(&b->lock);
        return b;
      }
    }
    if(i != h)
      release(&bcache.bucket[i].lock);
  }
  release(&bcache.bucket[h].lock);
  release(&bcache.evictlock);
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

  uint h = bhash(b->blockno);
  struct buf *head = &bcache.bucket[h].head;
  acquire(&bcache.bucket[h].lock);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = head->next;
    b->prev = head;
    head->next->prev = b;
    head->next = b;
  }
  
  release(&bcache.bucket[h].lock);
}

void
bpin(struct buf *b) {
  uint h = bhash(b->blockno);
  acquire(&bcache.bucket[h].lock);
  b->refcnt++;
  release(&bcache.bucket[h].lock);
}

void
bunpin(struct buf *b) {
  uint h = bhash(b->blockno);
  acquire(&bcache.bucket[h].lock);
  b->refcnt--;
  release(&bcache.bucket[h].lock);
}

