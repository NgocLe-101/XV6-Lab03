#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
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
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
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


  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
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
  uint64 start_virtual_address;
  int total_page;
  uint64 user_bitmask_address;

  argaddr(0, &start_virtual_address);   
  argint(1, &total_page);     
  argaddr(2, &user_bitmask_address); 

  if (start_virtual_address >= MAXVA || user_bitmask_address >= MAXVA) { 
      return -1;
  }

  if (total_page <= 0) {
      return -1;
  }
  
  struct proc *my_proc = myproc();
  int bitmask = 0;
  for (int i = 0; i < total_page; i++) {
      uint64 next_address = start_virtual_address + i * PGSIZE;
      pte_t *pte = walk(my_proc->pagetable, next_address, 0);
      if(pte == 0) {
          continue;
      }
      if ((*pte & PTE_V) && (*pte & PTE_A)) {
          bitmask |= (1 << i);
          *pte &= ~PTE_A;
      }
  }
  if (copyout(my_proc->pagetable, user_bitmask_address, (char *)&bitmask, sizeof(bitmask)) < 0) {
    return -1;
  }
  // lab pgtbl: your code here.
  return 0;
}
// #endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
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
