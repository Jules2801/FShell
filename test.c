#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "prompt.h"
#include "utils.h"
#include "command.h"




int main(int argc, char **argv){
    char *partition[] = {"ls", "-l", ";", "cd", "coucou", ";", "ftype", ".", ";", "echo", "bonjour", (char *) NULL};
    if(isStructuredCommand(partition)){
        int size;
        char ***res = makeStructuredCommand(partition, &size);

        for(int i = 0; i < size; i++){
            for(int j = 0; j < partSize(res[i]); j++){
                printf("%s ", res[i][j]);
            }
            puts("");
        }
    }

    exit(0);
}