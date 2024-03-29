#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "syscall.h"

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

extern int traceSyscalls;
extern int timesSysCallsUsed[totalSysCallNum];

int sys_toggle(void) {
  if (traceSyscalls == 1) {
    traceSyscalls = 0;
  } else {
		traceSyscalls = 1;
		for (int i=0; i<totalSysCallNum; i++) {
			timesSysCallsUsed[i] = 0;
    }
  }
  cprintf("trace set to %d\n", traceSyscalls);
  return 22;
}

int sys_print_count(void) {
char* syscalls[] = {
"sys_fork",
"sys_exit",
"sys_wait",
"sys_pipe",
"sys_read",
"sys_kill",
"sys_exec",
"sys_fstat",
"sys_chdir",
"sys_dup",
"sys_getpid",
"sys_sbrk",
"sys_sleep",
"sys_uptime",
"sys_open",
"sys_write",
"sys_mknod",
"sys_unlink",
"sys_link",
"sys_mkdir",
"sys_close",
"sys_toggle",
"sys_print_count",
"sys_add",
"sys_ps",
"sys_send",
"sys_recv",
};
  for (int i=0; i<totalSysCallNum; i++) {
		if (timesSysCallsUsed[i] != 0) {
			cprintf("%s %d\n",syscalls[i], timesSysCallsUsed[i]);
		}
	}
	return 23;
}

int sys_add(int a, int b) {
	argint(0,&a);
	argint(1,&b);
	int sum = a + b;
	return sum;
}

int sys_ps(void) {
	return ps();
}

int sys_send(int sender_pid, int rec_pid, void* msg) {
	if (argint(0, &sender_pid) < 0) {
		return -1;
	}
	if (argint(1, &rec_pid) < 0) {
		return -1;
	}
	char* char_msg = (char*) msg;
	if (argstr(2, &char_msg) < 0) {
		return -1;
	}
	return send(sender_pid, rec_pid, (void*) char_msg);
}

int sys_recv(void* msg) {
	char* char_msg = (char*) msg;
	if (argstr(0, &char_msg) < 0) {
		return -1;
	}
	return recv((void*)char_msg);
}