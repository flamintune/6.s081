# Lab2 System Calls

lab2 主要实现了两个系统调用

<syscall名字> <对应程序文件>
* trace(int n) `kernel/sysproc.c` 

    接受一个mask，来指定需要追踪的系统调用(系统调用号来标志)，实现思路如下

    1.在proc中加入一个mask字段，用来记录需要追踪的系统调用
    2.在syscall.c中每次处理系统调用的程序中，用与运算判断是否是对应的系统调用，如果是则打印输出该系统调用以及返回值

* sysinfo(sysinfo *info) `kernel/sys_info.h kernel/sysproc.c`

    接受一个sysinfo的结构体，并填充该结构体，返回空闲内存和进程数量，实现思路如下

    1.定义sysinfo的结构体
    2.添加sys_sysinfo的系统调用，在内核中，获取空闲内存和进程数量并拷贝到用户地址

    
