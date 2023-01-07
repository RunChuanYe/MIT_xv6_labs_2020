// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.
  
uint64 refer_count[((uint64)PHYSTOP - (uint64)KERNBASE) / (uint64)PGSIZE];
struct spinlock refer_count_lock;


#define VALID_RANGE(pa) ((pa) <= (uint64)PHYSTOP && (pa) >= (uint64)KERNBASE)
#define INDEX(pa) ((pa - (uint64)KERNBASE) / PGSIZE)

void set_value(uint64 pa, uint64 set_one);
uint64 get_ref_count(uint64 pa);
void change_ref_count(uint64 pa, int add);
void get_lock();
void release_lock();

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&refer_count_lock, "refer_count_lock");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)

void my_free(uint64 pa) {

  struct run *r;
  // Fill with junk to catch dangling refs.
  memset((void*)pa, 1, PGSIZE);
  set_value((uint64)pa, 0);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

void
kfree(void *pa)
{

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  get_lock();
  if (get_ref_count((uint64)pa) == 0)
    my_free((uint64)pa);
  else {
    change_ref_count((uint64)pa, 0);
    if (get_ref_count((uint64)pa) == 0)
      my_free((uint64)pa);
  } 
  release_lock();
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r) {
    memset((char*)r, 5, PGSIZE); // fill with junk
    // kalloc does not cause inter-process contention
    // They don't change the refer_count of the same pg at the same time
    // in kalloc func, so the lock is unnecessary
    // get_lock();
    set_value((uint64)r, 1);
    // release_lock();
  }
  return (void*)r;
}

// add : 0 --
// add : 1 ++
void change_ref_count(uint64 pa, int add) {
  if (VALID_RANGE(pa)) {
    if (add) 
      refer_count[INDEX(pa)]++;
    else 
      refer_count[INDEX(pa)]--;
  }
}

uint64 get_ref_count(uint64 pa) {
  if (VALID_RANGE(pa)) {
    uint64 ref = refer_count[INDEX(pa)];
    return ref;
  } else {
    return -1;
  }
}

void set_value(uint64 pa, uint64 set_one) {
  if (VALID_RANGE(pa)) {
    if (set_one)
      refer_count[INDEX(pa)] = 1;
    else 
      refer_count[INDEX(pa)] = 0;
  }
}

void get_lock(){
  if (!holding(&refer_count_lock))
    acquire(&refer_count_lock);
}

void release_lock(){
  if (holding(&refer_count_lock))
    release(&refer_count_lock);
}