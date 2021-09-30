#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


// #ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  // lab pgtbl: your code here.
  uint64 page_start;
  uint64 start, last;
  int num;
  pte_t *pte;
  uint64 result_addr;
  unsigned int result_bits = 0;
  if (argaddr(0, &page_start) < 0 || argint(1, &num) < 0 || argaddr(2, &result_addr) < 0)
  {
    return -1;
  }

  pagetable_t pagetable = myproc()->pagetable;
  // vmprint(pagetable);
  start = PGROUNDDOWN(page_start);
  last = PGROUNDDOWN(start + num * PGSIZE-1 );
  // printf("ppppppppppppppppp: start=%p  end=%p page_start=%p \n", start,start+PGSIZE, page_start);
  num=0;
  for (;;)
  {
    if ((pte = walk(pagetable, start, 0)) == 0)
      return -1;
    // printf("nnnnnnnnnnnnnnnnn: start=%p end=%p num=%d pte_num=%d pte_flag=%d %p flag=%d \n",start,start+PGSIZE, num,PX(0,start), PTE_FLAGS(*pte), *pte, (*pte & PTE_A));
    if (*pte & PTE_A)
    {
    // printf("nnnnnnnnnnnnnnnnn: start=%p end=%p num=%d pte_num=%d pte_flag=%d %p flag=%d \n",start,start+PGSIZE, num,PX(0,start), PTE_FLAGS(*pte), *pte, (*pte & PTE_A));
    result_bits = result_bits | 1 << num;
    //clear Accessed bit flag
    // *pte = *pte & 0xffffff9f;
    *pte = *pte >> 10 <<10|  (PTE_FLAGS(*pte) & 0b1110111111);  
    }
    num++;

    if (start == last)
      break;
    start += PGSIZE;
  }
  // printf("reult_bits: %d\n", result_bits);
  if (copyout(myproc()->pagetable, result_addr, (char *)&result_bits, sizeof(result_bits)) < 0)
    return -1;

  return 0;
}
// #endif

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
