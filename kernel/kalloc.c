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

struct run {
  struct run *next;
};

struct kmem {
  struct spinlock lock;
  struct run *freelist;
};

struct kmem kmems[NCPU];
void my_kfree(int cpu_id, void *pa);


void
kinit()
{
  for (int i = 0; i < NCPU; ++i)
    initlock(&(kmems[i].lock), "kmem_per_cpu");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  int index = 0;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    my_kfree((index++) % NCPU, p);
}

void my_kfree(int cpu_id, void *pa) {
  struct run* r;
  memset(pa, 1, PGSIZE);
  r = (struct run*)pa;
  acquire(&(kmems[cpu_id].lock));
  r->next = kmems[cpu_id].freelist;
  kmems[cpu_id].freelist = r;
  release(&(kmems[cpu_id].lock));
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  push_off();
  int curr_cup_id = cpuid();
  pop_off();

  acquire(&(kmems[curr_cup_id].lock));
  r->next = kmems[curr_cup_id].freelist;
  kmems[curr_cup_id].freelist = r;
  release(&(kmems[curr_cup_id].lock));
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  push_off();
  int curr_cup_id = cpuid();
  pop_off();

  acquire(&kmems[curr_cup_id].lock);
  r = kmems[curr_cup_id].freelist;
  if(r) 
    kmems[curr_cup_id].freelist = r->next;
  release(&kmems[curr_cup_id].lock);

  if (!r) {
    for (int i = 0; i < NCPU; ++i) {
      if (i == curr_cup_id) continue;
      else {
        acquire(&kmems[i].lock);
        r = kmems[i].freelist;
        if(r) {
          kmems[i].freelist = r->next;
          release(&kmems[i].lock);
          break;
        } 
        release(&kmems[i].lock);
      }
    }
  }

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
