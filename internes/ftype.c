#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

void cmd_ftype(char *ref) {
    struct stat statbuf;

    if (stat(ref, &statbuf) == -1) {
        perror("stat");
        return;
    }

    if (S_ISDIR(statbuf.st_mode)) {
        printf("directory\n");
    } else if (S_ISREG(statbuf.st_mode)) {
        printf("regular file\n");
    } else if (S_ISLNK(statbuf.st_mode)) {
        printf("symbolic link\n");
    } else if (S_ISFIFO(statbuf.st_mode)) {
        printf("named pipe\n");
    } else {
        printf("other\n");
    }
}