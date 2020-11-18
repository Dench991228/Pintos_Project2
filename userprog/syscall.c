#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"

#define NumberCall 21
#define stdin 1

typedef void (*handler)(struct intr_frame *f);

handler handlers[NumberCall];

static void syscall_handler (struct intr_frame *);

/* 用来应对halt系统调用 */
void SysHalt(struct intr_frame* );

/* 用来应对Exit系统调用 */
void SysExit(struct intr_frame* );

syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  handlers[SYS_HALT] = SysHalt;
}

static void
syscall_handler (struct intr_frame *f) 
{
  printf ("system call!\n");
  int call_number = getSyscallNumber(f);
  handlers[call_number](f);
  //thread_exit ();
}
/*用来判断系统调用给我的地址有没有问题*/
bool validateAddr(void* vaddr){
  return is_user_vaddr(vaddr);
}
/*用来获取一个系统调用的编号*/
int getSyscallNumber(struct intr_frame *f){
  return *((int*)f->esp);
}
/*用来获取一个系统调用的参数*/
void* getArguments(struct intr_frame *f, int pos){
  int* stack_top = f->esp;
  stack_top+=pos;
  return (*stack_top);
}

void SysHalt(struct intr_frame* f){
  shutdown_power_off();
  f->eax = 0;
}

void SysExit(struct intr_frame *f){
  f->eax = 0;
  thread_exit();
}