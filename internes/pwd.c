#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void cmd_pwd() {
    char cwd[1024];
    
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("getcwd");
    }
}