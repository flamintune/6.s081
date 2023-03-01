# lab9 mmap

lab9实现mmap和munmap两个系统调用，用来映射文件到地址空间

* mmap ``

    1.首先需要在用户的地址空间内，找到一块空闲的区域用来映射mmap页

    为了避免mmap的地址跟进程的地址发生冲突，我们将mmap映射到尽可能高的位置，离堆栈远点

    2.定义vma结构体，里面存放mmap映射的内存区域的必要信息，例如开始地址、文件大小、所映射文件、文件偏移、权限等等。并在proc中声明几个vma

    3.实现sys_mmap

    4.实现sys_unmap