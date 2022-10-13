#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
void func()
{
    int p;
    int f[2];
    int i;
    if (read(0,&p,sizeof(p)))
    {
        printf("prime %d\n",p);
        pipe(f);
        if (fork() == 0)
        {
            close(1);
            dup(f[1]); // 改变标准输出
            close(f[1]);
            close(f[0]);
            while (read(0,&i,sizeof(i)))
                if (i % p)
                    write(1,&i,sizeof(i));
            exit(0);
        }
        else
        {
            close(0);
            dup(f[0]); // 改变标准输入
            close(f[0]);
            close(f[1]);
            wait(0);
            func();
        }
    }
}
int main(int argc,char *argv[])
{
    int fd[2];
    int i;
    pipe(fd);

    if (fork() == 0)
    {
        close(fd[0]); // 实时手动释放不必要的资源
        for (i = 2;i <= 35;++ i)
            write(fd[1],&i,sizeof(i));
        close(fd[1]);
        exit(0);
    }
    else
    {
        close(0);
        dup(fd[0]);
        close(fd[0]);
        close(fd[1]);
        wait(0);
        func();
        
    }

    exit(0);
}