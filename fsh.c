#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "commandList.h"
#include "prompt.h"
#include "utils.h"
#include "command.h"
#include "glob.h"
#include "commandList.h"

char *previous_directory = NULL;
char *current_directory = NULL;
int isInRed = 0;
int isOutRed = 0;
int isErrRed = 0;



int main(int argc, char **argv){
    int error = 0;
    char *input;
    current_directory = getcwd(NULL,0);//On initalise le repertoire courant
    previous_directory = NULL;// Pour l'instant on a pas d'ancien repertoire


    while(1) {
        rl_outstream = stderr;
        char *prompt_string = prompt(error);
        input = readline(prompt_string);

        if(input == NULL) break;
        if(strlen(input) == 0) continue;
        if(*input) add_history(input);
        
        command *commande = newCommand(input);

        free(prompt_string);
        free(input);

        applyCommand(commande, &error);
        freeCommand(commande);
    }

    if(error < 0) exit(255);
    exit(error);
}