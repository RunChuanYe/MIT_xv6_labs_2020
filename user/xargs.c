#include "kernel/types.h"
#include "user/user.h"

#define BUF_SIZE 512


int main(int argc, char* argv[]) {

    int i;
    char buf[BUF_SIZE];
    char* execArgv[argc+1];
    for (i = 1; i < argc; ++i) {
        execArgv[i-1] = argv[i];
    }
    *(execArgv[argc]) = 0;
    while (gets(buf, BUF_SIZE)) {
        if (strlen(buf) == 0) 
            break;
        if (fork() == 0) {
            buf[strlen(buf) - 1] = '\0';
            execArgv[argc-1] = buf;
            exec(argv[1], execArgv);
            exit(0);
        }
        wait(&i);
    }    
    exit(0);
}