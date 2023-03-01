# lab3 page tables

本次lab主要涉及到页表，有下面三个任务

* 加速getpid()系统调用速度 `kernel/proc.c kernel/memlayout.h` 实现思路如下

    1.在进程创建的时候，为进程地址空间多映射一个只读的页，并且在这个页的开始处，存放一个usyscall结构体，用来存放当前进程的PID
  
    2.然后提供ugetpid()函数，它会获取usyscall页面上的结构体，然后返回pid，`return *(struct USYSCALL)->pid`,USYSCALL是分配的页的首地址

* 在系统初始化的时候打印页表 `kernel/vm.c` 实现思路如下

    1.设计页表的函数 vmprint() 在exec中调用vmprint，当当前进程为1时，调用
    
    2.vmprint函数的实现，xv6的页表设计是三级页表，每一级有512个pte，上一级的pte的pnn会指向下一级页表的地址，最后一级的pnn会指向物理地址。很显然这里可以用递归来实现，我们遍历每一级页表，当pte有效，就打印输出该pte，当pte有效且能读能写能被执行说明它的pnn指向下一级页表，我们递归传入新的页表地址，以及级别加1

    ```c
    void vmprint(pagetable_t pagetable,int index){
    // there are 2^9 = 512 PTEs in a page table.
        if (!index)
            printf("page table %p\n",pagetable);
        for(int i = 0; i < 512; i++){
            pte_t pte = pagetable[i];
            
            if(pte & PTE_V){

            for (int j = 0;j < index;++ j)
                printf(".. ");
            printf("..%d: pte %p pa %p\n",i,pte,PTE2PA(pte));
            }
            if((pte & PTE_V) && (pte & (PTE_R|PTE_W|PTE_X)) == 0){
                // this PTE points to a lower-level page table.
            uint64 child = PTE2PA(pte); // 把后10位的falg去掉，然后填充0，通过移位操作完成
                vmprint((pagetable_t)child,index+1);
            }
        }
    }
    ```

* 实现pgaccess()，用来报告哪些页面被访问了的 `kernel/vm.c` 下面是实现思路

    1.虽然xv6是运行qemu上的，但是对于访问的页仍然会被MMU去在PTE上标记对应位，定义PTE_A，它是标识页面是否被访问的标记，然后我们遍历给出的页范围，看它的PTE是否有PTE_A标识，如果有则在传入的bitmask中标记上
    
    2.`pgaccess(pagetable_t pagetable,uint64 va,int number,uint* bitmask)`  va为虚拟地址，number为页的数量。
     
