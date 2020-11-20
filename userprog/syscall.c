#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "pagedir.h"
#include "filesys/filesys.h"
#include "filesys/file.h"

#define NumberCall 21
#define stdin 1

typedef void (*handler)(struct intr_frame *f);

handler handlers[NumberCall];
/*获得系统调用的编号*/
int getSyscallNumber(struct intr_frame *f);
/*获得pos处的系统调用参数*/
void* getArguments(struct intr_frame *f, int pos);
/*判断一个地址有没有问题*/
bool validateAddr(void* vaddr);

static void syscall_handler (struct intr_frame *);

/* 用来应对halt系统调用 */
void SysHalt(struct intr_frame* );

/* 用来应对Exit系统调用 */
void SysExit(struct intr_frame* );

/* 用来应对Write系统调用*/
void SysWrite(struct intr_frame*);

/*用来应对create系统调用*/
void SysCreate(struct intr_frame *f);

void syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  handlers[SYS_HALT] = SysHalt;
  handlers[SYS_WRITE] = SysWrite;
  handlers[SYS_EXIT] = SysExit;
  handlers[SYS_CREATE] = SysCreate;
}

static void
syscall_handler (struct intr_frame *f) 
{
  //printf ("system call!\n");
  //shutdown_power_off();
  validateAddr((const void*)f->esp);
  int call_number = getSyscallNumber(f);
  handlers[call_number](f);
  //thread_exit();
}

/*用来判断系统调用给我的地址有没有问题*/
bool validateAddr(void* vaddr){
  if(vaddr!=NULL&&vaddr<PHYS_BASE&&is_user_vaddr(vaddr)&&pagedir_get_page(thread_current()->pagedir, vaddr)){
    return true;
  }
  else{
    exit(-1);
  }
}

/*用来获取一个系统调用的编号*/
int getSyscallNumber(struct intr_frame *f){
  return *((int*)f->esp);
}

/*用来获取一个系统调用的参数*/
void* getArguments(struct intr_frame *f, int pos){
  validateAddr((int*)f->esp+pos);
  int* stack_top = f->esp;
  stack_top+=pos;
  return (void*)(*stack_top);
}

/*关机系统调用,没有参数*/
void SysHalt(struct intr_frame* f){
  //puts("HALT");
  shutdown_power_off();
  //f->eax = 0;
}

/*退出进程系统调用*/
/*int*/
void SysExit(struct intr_frame *f){
  int ret_value = getArguments(f, 1);
  printf("%s: exit(%d)\n", thread_current()->name, ret_value);
  //struct thread *cur = thread_current();
  //cur->ret = ret_value;
  thread_current()->ret = ret_value;
  f->eax = 0;
  thread_exit();
}

/*写文件系统调用*/
/*int fd, char *buffer, unsigned size*/
void SysWrite(struct intr_frame *f){
  int *esp = f->esp;
  int fd = (int)getArguments(f,2);
  const char* buffer = (const char*)*(esp+6);
  unsigned size = (size_t)getArguments(f,3);
  //printf("file descriptor:%d, second arguments:%x, buffer size:%d, fourth arguments:%d, fifth arguments:%d, sixth argument:%d\n", fd, buffer,size,*(esp+4), *(esp+5), *(esp+6));
  putbuf(buffer, size);
  f->eax=0;
}

/*创建一个新文件*/
/*char* file_name, unsigned initial_size*/
void SysCreate(struct intr_frame *f){
  const char* file_name = (char*)getArguments(f,4);
  validateAddr(file_name);
  unsigned initial_size = getArguments(f,2);
  //printf("file_name:%s\n", file_name);
  bool ok = filesys_create(file_name, initial_size);
  f->eax = (int)ok;
}

/*从一个进程中退出*/
void exit(int status){
  printf("%s: exit(%d)\n", thread_current()->name, status);
  thread_current()->ret = status;
  thread_exit();
}
