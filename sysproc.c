#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "psched.h"
#include "spinlock.h"

struct ptable {
  struct spinlock lock;
  struct proc proc[NPROC];
} ;

extern struct ptable ptable;

int
sys_nice(int n)
{
  int old_nice;
  struct proc *curproc = myproc();

  if(argint(0, &n) < 0 || n < 0 || n > 20) {
    return -1; // Return -1 on error (invalid argument or out of bounds nice value)
  }

  old_nice = curproc->nice;
  curproc->nice = n;

  return old_nice;
}

int
sys_getschedstate(struct pschedinfo *psi)
{
  if(argptr(0, (void*)&psi, sizeof(*psi)) < 0)
    return -1; // return -1 if failed to get argument

  if(psi == 0) {
    return -1; // NULL pointer
  }

  
  for(int i = 0; i < NPROC; ++i) {
    psi->inuse[i] = (ptable.proc[i].state != UNUSED);
    //cprintf("Psi->inuse[%d] = %d\n", i, psi->inuse[i]);
    psi->priority[i] = ptable.proc[i].priority;
    //cprintf("Psi->priority[%d] = %d\n", i, psi->priority[i]);
    psi->nice[i] = ptable.proc[i].nice;
    //cprintf("Psi->nice[%d] = %d\n", i, psi->nice[i]);
    psi->pid[i] = ptable.proc[i].pid;
    //cprintf("Psi->pid[%d] = %d\n", i, psi->pid[i]);
    psi->ticks[i] = ptable.proc[i].cpu;
    //cprintf("Psi->ticks[%d] = %d\n\n", i, psi->ticks[i]);


  }

  return 0; // Success
}

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
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

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;

  myproc()->ticksleft = ticks + n;

  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  myproc()->ticksleft = 0;
  myproc()->cpu += n;
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}




