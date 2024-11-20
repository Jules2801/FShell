#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "utils.h"
#include "command.h"


int main(int argc, char **argv){
    int errorCount = 0;
    char *input;


    while(1) {
        input = readline("./fsh : ");

        if(input == NULL) break;
        if(*input) add_history(input);
        
        char **command = split_command(input);

        if((applyCommand(command)) >= 1) errorCount++;
    }
}