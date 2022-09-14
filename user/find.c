#include "kernel/types.h"
#include "user/user.h"
#include "kernel/stat.h"
#include "kernel/fs.h"

#define stderr 2
#define RM_PATH(str) (strcmp(str, ".") == 0 || strcmp(str, "..") == 0)

char* fmtname(char *path) {
    char *p;
    // Find first character after last slash.
    for(p=path+strlen(path); p >= path && *p != '/'; p--) ;
    return ++p; 
}

void find(char* path, char* fileName) {
    int fd;
    char buf[512], *p;
    struct dirent de;
    struct stat st;
    if((fd = open(path, 0)) < 0){
        fprintf(stderr, "find: cannot open %s\n", path);
        return;
    }
    if(fstat(fd, &st) < 0){
        fprintf(stderr, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }
    
  switch(st.type){
    case T_FILE:
        if (strcmp(fmtname(path), fileName) == 0)
            printf("%s\n", path);
        break;
    case T_DIR:
        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
            printf("find: path too long\n");
            break;
        }
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';
        while(read(fd, &de, sizeof(de)) == sizeof(de)){
            if(de.inum == 0 || RM_PATH(de.name))
                continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if(stat(buf, &st) < 0){
                printf("find: cannot stat %s\n", buf);
                continue;
            }
            find(buf, fileName);
        }
        break;
    }
    close(fd);
    return;    
}

int main(int argc, char* argv[]) {

    if (argc < 3) {
        fprintf(2, "find: expected 2 arguments, but got %d\n", (argc-1));
        exit(-1);
    }
    find(argv[1], argv[2]);

    exit(0);
}