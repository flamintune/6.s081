#include <kernel/types.h>
#include <kernel/stat.h>
#include <user/user.h>

int main(int argc,char *argv[])
{
    int fd[2];
    char buf[1024];
    if (pipe(fd) == -1) // 生成 pipeline
    {
        fprintf(2,"pipe fail");
        exit(1);
    }
    if (fork() == 0 )
    {
        close(fd[0]);
        write(fd[1],"1",4);
        close(fd[1]);
        printf("%d: received ping\n",getpid());
    }
    else
    { 
        close(fd[0]);
        read(fd[0],buf,4);
        close(fd[1]);
        wait(0);
        printf("%d: received pong\n",getpid());
    }
    exit(0);
}