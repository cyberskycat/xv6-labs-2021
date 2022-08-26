// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

void freerange(void *pa_start, void *pa_end,int cpu_index);
void kfree_freelist(void *pa,int cpu_index);
extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct kmem{
  struct spinlock lock;
  struct run *freelist;
};
struct kmem kms[NCPU];
void
kinit()
{
  uint64 start = (uint64)end;
  uint64 range = (uint64)PHYSTOP - start; 
  uint64 pre_cpu_range = range / NCPU;
  for(int i=0;i<NCPU;i++){
    // printf("bbbbbbbbbb% start=%p  phystop=%p\n",start,PHYSTOP);
    initlock(&kms[i].lock, "kmem");
    freerange((void *)start,(void *)start+pre_cpu_range,i);
    start = start + pre_cpu_range;
  }
    // printf("bbbbbbbbbb% start=%p  phystop=%p\n",start,PHYSTOP);

}

void
freerange(void *pa_start, void *pa_end,int cpu_index)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  int n = 0;
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
     n++;
    kfree_freelist(p,cpu_index);
  }
  // printf("hhhhhhhhhhhhhhhhh n=%d\n",n);
}

void
kfree_freelist(void *pa,int cpu_index)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree_freelist");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kms[cpu_index].lock);
  r->next = kms[cpu_index].freelist;
  kms[cpu_index].freelist = r;
  release(&kms[cpu_index].lock);
}
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
int get_cpu_id(){
    push_off();
    int id = cpuid();
    pop_off();
    return id;
}

void kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;
  int cpuid = get_cpu_id();
  acquire(&kms[cpuid].lock);
  r->next = kms[cpuid].freelist;
  kms[cpuid].freelist = r;

  release(&kms[cpuid].lock);

}

struct run* steal_from_other(int mycpuid){
  struct run *r;
  r=(void *)0;
  for(int i=0;i<NCPU;i++){
    if(mycpuid == i)continue;
     acquire(&kms[i].lock);
     r = kms[i].freelist;
     if(r){
       kms[i].freelist = r->next;
     }else{
          release(&kms[i].lock);
          continue;
     }
     release(&kms[i].lock);
     break;
  }
  return r;
}
// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void * kalloc(void)
{
  struct run *r;
  int cpuid = get_cpu_id();
  acquire(&kms[cpuid].lock);
  r = kms[cpuid].freelist;
  if(r){
    kms[cpuid].freelist = r->next;
  }
  release(&kms[cpuid].lock);
  if(!r){
    r = steal_from_other(cpuid);
    
    if (!r){
        // printf("steal fail!!!!! cpuid=%d\n",cpuid);
        r = steal_from_other(cpuid);
      }{
        // printf("steal success!!!!!\n");

      }
  }
  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
