#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
// 问题关键在如何存储argv 和 标准输入传过来的文件
// 将标准输入转到命令行参数
char *readline()
{
    char *buf = (char *)malloc(20);
    char *pbuf = buf;
    while (read(0,pbuf,1))
    {
        if (*pbuf == ' ' || *pbuf == '\n')
        {
            *pbuf = '\0';
            return buf;
        }
        pbuf++;        
    }
    // 判断最后一次
    if (pbuf != buf)
        return buf;
    
    free(buf);
    return 0;
}
int main(int argc,char *argv[])
{   
    // 0 stdin 标准输入
    // argv 通过参数传递进来的输入
    // fork + exec
    // exec 执行时的参数 以及 标准输入
    if (argc < 2)
    {
        fprintf(2,"less arguments\n");
        exit(-1);
    }

    char *newargv[20];
    
    char **pargv = argv;
    pargv++;

    char **pnewargv = newargv; // 指针作为指向来访问
    
    while(*pargv) // 复制 argv
        *pnewargv++ = *pargv++;
    
    // 复制 stdin
    // 1.不知道一次 read 多少个 字节
    // 2.空格似乎也会算到 标准输入里面
    // 3.换行字符 /n 也是如此
    // 一个一个读的时候如何转换成字符串
    // 要深刻去理解 指针，read 是读到指针所指向的位置里面去，所以这里我们直接用一个指针指向 newargv就好了
    // 理解了 char*[] 本质就是存储了指针的数组！所以还是要搞个动态分配这种，前面的赋值just把引用都指向了同一个地方罢了未
    // char *buf = (char*)malloc(100);
    char *pbuf;
    int newargc = argc - 1;
    while ((pbuf = readline()) != 0)
    {
        *pnewargv++ = pbuf;
        newargc ++;
    }
    newargv[newargc] = 0;
    if (fork() == 0)
    {
        exec(argv[1],newargv);
    }
    else
    {
        wait(0);
    }

    exit(0);
}