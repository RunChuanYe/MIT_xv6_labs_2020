#include "kernel/types.h"
#include "user/user.h"

#ifndef PINGPONG_BUF_SIZE
#define PINGPONG_BUF_SIZE 255
#endif

int main(int args, char* argv[]) {

    int par_read_chi_write[2];
    int chi_read_par_write[2];
    char buf[PINGPONG_BUF_SIZE];
    int msgLen = 4;    
    char* msg1 = "ping";
    char* msg2 = "pong";

    if (pipe(par_read_chi_write) < 0) exit(1);
    if (pipe(chi_read_par_write) < 0) exit(1);

    int ret = fork();
    if (ret < 0) {
        printf("fork error\n");
        exit(-1);
    }

    if (ret == 0) {
        close(par_read_chi_write[0]);
        close(chi_read_par_write[1]);
        if (read(chi_read_par_write[0], buf, msgLen)) {
            printf("%d: received %s\n", getpid(), buf);
            close(chi_read_par_write[0]);
            write(par_read_chi_write[1], msg2, strlen(msg2));
            close(par_read_chi_write[1]);
        }
        exit(0);
    } else {
        close(chi_read_par_write[0]);
        close(par_read_chi_write[1]);
        write(chi_read_par_write[1], msg1, msgLen);
        close(chi_read_par_write[1]);
        if (read(par_read_chi_write[0], buf, msgLen)) {
            printf("%d: received %s\n",getpid(), buf);
            close(par_read_chi_write[0]);
        }
    }
    wait(&ret);
    exit(0);
}