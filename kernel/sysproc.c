#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
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

void copy_userpg_to_kernelpg(pagetable_t user, pagetable_t kernel);

uint64
sys_sbrk(void)
{
  int n;

  if(argint(0, &n) < 0)
    return -1;

  struct proc * p = myproc();
  uint64 addr = p->sz;

  if(growproc(n) < 0)
    return -1;

  // // copy userpg to kernelpg
  // uint64 userpg = PTE2PA(p->pagetable[0]);
  // uint64 kernelpg = PTE2PA(p->kernel_pgtbl[0]);
  // copy_userpg_to_kernelpg((pagetable_t)userpg, (pagetable_t)kernelpg);

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

uint64
sys_checkvm()
{
  return (uint64)test_pagetable(); 
}