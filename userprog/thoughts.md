# 关于用户内存访问问题
## 系统需求
- 系统调用的时候,不能有"问题指针"
- 问题指针包括如下内容
    - 空指针
    - 没有被映射的虚拟地址
    - 指向内核空间的虚拟地址
- 遇到"问题指针"怎么办:消灭这个进程
## 解决方案
- 先验证一下这个传进来的指针有没有啥问题,然后再使用它
- 参考内容:
    - threads/vaddr.h:is_user_vaddr,is_kernel_vaddr
    - userprog/pagedir.c:
# 系统调用问题
- 如何传递参数:
    - 通过handler中的intr_frame传递
        - 编号:esp指向的数
        - 参数:esp之上的各种数
## Exit调用(已完成)
### 不考虑非法退出的情况
1. 把eax设定为返回值
2. thread_exit()即可
### 考虑非法退出
#### 数据结构改变
- thread:添加ret用来保存返回值,必须是整数
#### 具体步骤
1. 从intr_frame中获取返回值
2. 把这个值放到当前线程的ret中
3. 正常退出,所以eax是0
4. thread_exit()
### 完成真正的退出
- 修改thread结构体:
    - 作为父进程:
        - wait_for:当前进程在等待那个进程
        - child_sema:因为等待某个子进程,所以需要有一个信号量
        - children_list:子进程列表
    - 作为子进程:
        - father:父进程
        - struct list_elem
    - 本质属性:
        - ret:返回值
- 业务逻辑:
    - Process_Wait检查被等待的线程是不是已经DYING
        - 如果是,返回它的返回值
        - 如果不是,拿信号量,等着子进程来解锁
    - init_thread():需要考虑子进程列表,父进程,等各项属性
    - thread_exit():看一眼父进程在没在等,在等就唤醒一下

## Halt调用(已完成)
## Create调用(完成,没什么难度)
## Open调用(已完成)
### 对于thread结构体的改变
- max_fd:当前最大的文件描述符,初始是1,之后总是比它大一,不管关不关文件都不变它
- list_file:一个进程打开的文件
### 新增结构体
```C
struct opened_file{
    struct file* position;
    int fd;
    struct list_elem elem;
};
```
## Close调用(已完成)
- 涉及到从一个进程的文件列表中根据文件描述符找到相关的文件,并且将其删除
- 如果fd是1或者0,就不管他
## Read调用
## Write调用
## Filesize调用
## Tell调用
## Remove调用