# 6.s081(xv6)
xv6是MIT6.s081这门操作系统课所用到的基于riscv架构的教学操作系统。这门课，主要布置了以下10个lab，这些lab基本上都是围绕xv6来修改相关代码，以及增加相关特性

## 写在前面
----
项目是以一个git分支来完成一个lab的，所以不同的lab分布在不同的分支中。

## 目录
labx: \<labname\> \<branchname\>
- lab1: Utilities   util

    lab1主要是利用系统调用实现一些小命令，有sleep、find、primes、xargs。
    其中稍为绕一点的是primes，基于管道来实现素数筛算法

- lab2: SystemCalls syscall

    lab2主要是熟悉xv6的系统调用的工作方式，为xv6添加两个系统调用
    1.sys_trace() 记录一个进程都使用了哪些系统调用，并输出
    
    2.sys_info() 收集正在运行的系统信息(空闲内存，进程数量)存在一个结构体sysinfo中


- lab3: PageTables  pgtbl

    lab3主要是对页表的一些操作，包括以下内容
    
    1.通过修改页表映射加速getpid()系统调用，让其无需经过内核
    
    2.实现一个函数vmprint()，用来打印当前页表的映射情况
    
    3.实现一个新的系统调用pgaccess()，检测并报告被访问的页

- lab4: traps       traps

    lab4主要是探究基于trap的系统调用的过程，包含以下内容
    
    1.实现函数backtrace()，打印栈上的函数调用情况
    
    2.实现系统调用sigalarm(n,fn),每过n个tick调用函数fn、以及系统调用sigreturn(void)，用来重启用户程序


- lab5: cow         cow

    lab5主要是实现cow(copy on write)写时复制这个特性
    
    在传统fork中，因为需要拷贝父进程的地址空间，但有时其实用不到父进程的比如子进程要执行exec的时候，这样就会浪费了拷贝时间。
    
    于是引入了cow这个技术，先不急着给子进程分配物理页，即父子进程都指向相同的物理页，当需要写的时候，利用pagefault机制再来分配新的物理页。


- lab6: mutilthread thread

    lab6主要与多线程有关，大致内容为实现一个简单的线程库，以及用pthread库写一些简单的多线程程序
    
    1.实现线程切换、创建线程
    
    2.减小锁的颗粒度来提高程序的并发速度
    
    3.利用pthread库实现多线程的同步

    lab2    
- lab7: locks       lock

    lab7主要是重新设计锁来提升并行度
    
    1.CPU的内存分配器
    
    2.Buffer cache 在多进程对于文件系统的读取中，对于Buffer cache代码的重构

- lab8: filesystem  fs

    lab8主要是在文件系统中添加对大文件的支持，以及软链接
    
    1.提高文件的最大大小
    
    2.实现软链接

- lab9: mmap        mmap

    lab9主要是实现mmap系统调用

- lab10: networking  net

    lab10主要是写一个简单的xv6网络驱动程序
