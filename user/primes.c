#include "kernel/types.h"
#include "user/user.h"

void prime(int fd) {
    char value, firstPrime;
    int p[2], ret;
    
    if(pipe(p) < 0) exit(-1);

    if (read(fd, &firstPrime, 1)) {
        printf("prime %d\n", firstPrime);
        while(read(fd, &value, 1)) {
            if (value % firstPrime != 0) {
                write(p[1], &value, 1);
            }
        }
    } else {
        exit(0);
    }
    close(fd);
    close(p[1]);
    if (fork() == 0) {
        prime(p[0]);
    }
    close(p[0]);
    wait(&ret);
    exit(0);
}


int main(int argc, char* argv[]) {

    int p[2];
    int ret;
    if(pipe(p) < 0) exit(-1);
    for (int i = 2; i < 36; ++i) {
        write(p[1], &i, 1);
    }
    close(p[1]);
    if (fork() == 0) {
        prime(p[0]);
    } 
    close(p[0]);
    wait(&ret);
    exit(0);
}