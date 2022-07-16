#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
// find <pathname> <name>
void find(char* path,char *dst)
{
    char buf[DIRSIZ + 1];
    char *p;
    
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path,0)) < 0)
    {
        fprintf(2,"find: cannot open %s\n",path);
        return ;
    }
    fstat(fd,&st);
    switch (st.type)
    {
        case T_FILE:
            fprintf(2,"%s not a directory!",path);
            break;
        case T_DIR:
            strcpy(buf,path); // path -> buf
            p = buf + strlen(buf); // p 设置到名字末尾
            *p++ = '/'; // buf 搞成文件目录名
            while(read(fd,&de,sizeof(de)) == sizeof(de)){ // 遍历目录下的所有文件
                if (de.inum == 0) // 节点个数
                    continue;
                // fprintf(1,"%s\n",de.name);
                if (strcmp(de.name,dst) == 0)
                    fprintf(1,"%s/%s\n",path,de.name);
                memmove(p,de.name,DIRSIZ);
                if (stat(buf,&st) < 0)
                {
                    printf("find: cannot stat %s\n",buf);
                    continue;
                }
                if (st.type == T_DIR && strcmp(de.name,".") && strcmp(de.name,".."))
                {   
                    char temp[512];
                    strcpy(temp,buf);
                     find(temp,dst); // buf 是指针，要克隆一个传进去捏
                }
            }
            break;

    }

}
int main(int argc,char* argv[])
{
    if (argc <= 1)
    {
        fprintf(2,"Usage: find <filename>...");
        exit(0);
    }
    if (argc <= 2)
    {
        find(".",argv[1]);      
    }
    if (argc >= 3)
    {
        find(argv[1],argv[2]);
    }
    exit(0);
}