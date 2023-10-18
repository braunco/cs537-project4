#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "psched.h"

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

int
sys_nice(void)
{
  int n;
  if(argint(0, &n) < 0)  // Fetch the integer argument of the syscall
    return -1;

  if(n < 0 || n > 20)  // Check if the nice value is within the valid range
    return -1;

  struct proc *curproc = myproc();
  int old_nice = curproc->nice;
  curproc->nice = n;

  return old_nice;  // Return the old nice value
}

int
sys_getschedstate(void)
{
  struct pschedinfo *psi;
  struct proc *p;
  int i = 0;

  // Fetch the pointer to the pschedinfo structure from the user space.
  if(argptr(0, (void*)&psi, sizeof(*psi)) < 0)
    return -1;

  // Acquire the ptable lock to safely access process information.
  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++, i++) {
    psi->inuse[i] = (p->state != UNUSED);
    psi->priority[i] = p->priority;
    psi->nice[i] = p->nice;
    psi->pid[i] = p->pid;
    psi->ticks[i] = p->cpu;  

  // Release the ptable lock.
  release(&ptable.lock);

  return 0;
}