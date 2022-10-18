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
  struct spinlock lock;
  struct buf buf[NBUF];
  struct spinlock bucket[NBUCKET];
  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  // struct buf head;
} bcache;
struct entry{
  struct buf* buf;
  struct entry *next;
};
struct entry *table[NBUCKET];
struct spinlock bcachelock[NBUCKET];
void
binit(void)
{
  struct buf *b;
  initlock(&bcache.lock,"bcache");
  for (int i = 0;i < NBUCKET;++ i)
    initlock(&bcache.bucket[i], "bcache");

  // Create linked list of buffers just a 尾叉法
  // bcache.head.prev = &bcache.head;
  // bcache.head.next = &bcache.head;
  // for(b = bcache.buf; b < bcache.buf+NBUF; b++){
  //   b->next = bcache.head.next;
  //   b->prev = &bcache.head;
  //   initsleeplock(&b->lock, "buffer");
  //   bcache.head.next->prev = b;
  //   bcache.head.next = b;
  // }

}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  acquire(&bcache.lock);
  int i = blockno % NBUCKET;
  // Is the block already cached?
  acquire(&bcache.bucket[i]);
  for (struct entry e = table[i];e;e = e->next)
    if (b->dev == dev){
      b->refcnt++;
      release(&bcache.bucket[i]);
      return b;
    }

  for(b = bcache.head.next; b != &bcache.head; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    if (b->refcnt == 0){
      b->dev = dev;
      b->blockno = blockno;
      strcut entry* p = table[i];
      while (p->next)
        p = p->next;
      p->next = 
      b->valid = 0;
      b->refcnt = 1;
      b->timestamp = 0;
      release(&bcache.bucket[i]);
      acquiresleep(&b->lock);
      return b;
    }
  }
  // for(b = bcache.head.prev; b != &bcache.head; b = b->prev){
  //   if(b->refcnt == 0) {
  //     b->dev = dev;
  //     b->blockno = blockno;
  //     b->valid = 0;
  //     b->refcnt = 1;
  //     release(&bcache.lock);
  //     acquiresleep(&b->lock);
  //     return b;
  //   }
  // }
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
  int i = b->blockno % NBUCKET;
  // acquire(&bcache.lock);
  acquire(&bcache.bucket[i]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    // 把 b 放到 head 后面
    b->timestamp = 0;
    // b->next->prev = b->prev;
    // b->prev->next = b->next;
    // b->next = bcache.head.next;
    // b->prev = &bcache.head;
    // bcache.head.next->prev = b;
    // bcache.head.next = b;
  }
  release(&bcache.bucket[i]);
  // release(&bcache.lock);
}

void
bpin(struct buf *b) {`
  acquire(&bcache.lock);
  b->refcnt++;
  release(&bcache.lock);
}

void
bunpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt--;
  release(&bcache.lock);
}


