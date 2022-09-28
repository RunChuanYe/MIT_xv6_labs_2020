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

#define NBUCKETS 13

struct {
  struct spinlock lock[NBUCKETS];
  struct buf buf[NBUF];
  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct buf hashbuckets[NBUCKETS];
} bcache;

void
binit(void)
{
  struct buf *b;
  int count = 0;

  for (int i = 0; i < NBUCKETS; ++i) {
    // Create linked list of buffers 
    initlock(&(bcache.lock[i]), "bcache_hash");
    bcache.hashbuckets[i].prev = &(bcache.hashbuckets[i]);
    bcache.hashbuckets[i].next = &(bcache.hashbuckets[i]);
  }

  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    int index = (count++) % NBUCKETS;
    b->next = bcache.hashbuckets[index].next;
    b->prev = &bcache.hashbuckets[index];
    initsleeplock(&b->lock, "buffer");
    bcache.hashbuckets[index].next->prev = b;
    bcache.hashbuckets[index].next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b, *try2;

  int bucket_index = blockno % NBUCKETS;

  acquire(&bcache.lock[bucket_index]);

  // Is the block already cached?
  for(b = bcache.hashbuckets[bucket_index].next; b != &bcache.hashbuckets[bucket_index]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock[bucket_index]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  for(b = bcache.hashbuckets[bucket_index].prev; b != &bcache.hashbuckets[bucket_index]; b = b->prev){
    if(b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      release(&bcache.lock[bucket_index]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  release(&bcache.lock[bucket_index]);

  for (int i = 0; i < NBUCKETS; ++i) {
    if (i == bucket_index) continue;
    else {
      acquire(&bcache.lock[i]);
      for(b = bcache.hashbuckets[i].prev; b != &bcache.hashbuckets[i]; b = b->prev){
        if(b->refcnt == 0) {
          b->prev->next = b->next;
          b->next->prev = b->prev;
          release(&bcache.lock[i]);
          acquire(&bcache.lock[bucket_index]);
          for(try2 = bcache.hashbuckets[bucket_index].next; try2 != &bcache.hashbuckets[bucket_index]; try2 = try2->next){
            if(try2->dev == dev && try2->blockno == blockno){
              try2->refcnt++;
              b->next = &bcache.hashbuckets[bucket_index];
              b->prev = bcache.hashbuckets[bucket_index].prev;
              bcache.hashbuckets[bucket_index].prev->next = b;
              bcache.hashbuckets[bucket_index].prev = b;
              release(&bcache.lock[bucket_index]);
              acquiresleep(&try2->lock);
              return try2;
            }
          }
          b->dev = dev;
          b->blockno = blockno;
          b->valid = 0;
          b->refcnt = 1;
          b->prev = &bcache.hashbuckets[bucket_index];
          b->next = bcache.hashbuckets[bucket_index].next;
          bcache.hashbuckets[bucket_index].next->prev = b;
          bcache.hashbuckets[bucket_index].next = b;
          release(&bcache.lock[bucket_index]);
          acquiresleep(&b->lock);
          return b;
        }
      }
      release(&bcache.lock[i]);
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

  int buckets_index = b->blockno % NBUCKETS;
  releasesleep(&b->lock);

  acquire(&bcache.lock[buckets_index]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.hashbuckets[buckets_index].next;
    b->prev = &bcache.hashbuckets[buckets_index];
    bcache.hashbuckets[buckets_index].next->prev = b;
    bcache.hashbuckets[buckets_index].next = b;
  }
  
  release(&bcache.lock[buckets_index]);
}

void
bpin(struct buf *b) {
  int buckets_index = b->blockno % NBUCKETS;
  acquire(&bcache.lock[buckets_index]);
  b->refcnt++;
  release(&bcache.lock[buckets_index]);
}

void
bunpin(struct buf *b) {
  int buckets_index = b->blockno % NBUCKETS;
  acquire(&bcache.lock[buckets_index]);
  b->refcnt--;
  release(&bcache.lock[buckets_index]);
}


