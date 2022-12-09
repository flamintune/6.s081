#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void){
    char *mem = (char *)malloc(sizeof(char) * 1000);
    *mem = 1;
    return 0;
}