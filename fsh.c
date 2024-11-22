#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/_types/_null.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "prompt.h"
#include "utils.h"
#include "command.h"
#include "glob.h"

char *previous_directory = NULL;
char *current_directory = NULL;



int main(int argc, char **argv){
    int error = 0;
    char *input;
    current_directory = getcwd(NULL,0);//On initalise le repertoire courant
    previous_directory = NULL;// Pour l'instant on a pas d'ancien repertoire


    while(1) {
        rl_outstream = stderr;
        input = readline(prompt(error));

        if(input == NULL) break;
        if(strlen(input) == 0) continue;
        if(*input) add_history(input);
        
        char **command = split_command(input);

        applyCommand(command, &error);

    }
}