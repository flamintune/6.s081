#include <kernel/types.h>
#include <kernel/stat.h>
#include <user/user.h>

int main(int argc,char *argv[])
{
    if (argc <= 1)
    {
        fprintf(2,"Usage: sleep seconds...\n");
        exit(1);
    }
    if (argc <= 2)
    {
        int n = atoi(argv[1]);
        sleep(n);
        exit(0);
    }
    if (argc >= 3)
    {
        fprintf(2,"Usage: sleep <seconds>\n");
        exit(1);
    }
    exit(0);
}