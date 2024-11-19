#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>
#include "utils.h"

int main(int argc, char **argv){
    char *errorMessage = "usage: for F in REP [-A] [-r] [-e EXT] [-t TYPE] [-p MAX] { CMD }\n";
    if(argc < 7 || strcmp("F", argv[1]) != 0 || strcmp("in", argv[2])){
        write(2, errorMessage, strlen(errorMessage));
        exit(1);
    }

    DIR *dir;
    if((dir = opendir(argv[3])) == NULL){
        perror("Erreur lors de l'ouverture du fichier");
        exit(1);
    }

    int a_option = 0;
    int r_option = 0;
    char *extension = NULL;
    char type;
    int max_parralel = -1;

    char *s = "Erreur : option inconnue ou argument manquant\n";

    int opt;
    while((opt = getopt(argc, argv+3, ":Are:t:p:")) != -1){
        switch(opt){
            case 'A':
                a_option = 1;
                fflush(stdout);
                break;

            case 'r':
                r_option = 1;
                break;

            case 'e' :
                if((extension = malloc(strlen(optarg) * sizeof(char) + 1)) == NULL){
                    char *s = "Erreur lors de l'allocation mémoire\n";
                    write(STDERR_FILENO, s, strlen(s));
                    exit(1);
                }

                strcpy(extension, optarg);
                *(extension + strlen(optarg)) = '\0';

                break;

            case 't' :
                if(strlen(optarg) != 1){
                    char *s = "Vous devez fournir un caractère\n";
                    write(STDERR_FILENO, s, strlen(s));
                    exit(1);
                }

                type = *(optarg);

                break;
            
            case 'p' :
                // TODO : Créer une fonction permettant de vérifier qu'on entre bien un entier
                max_parralel = atoi(optarg);

                break;

            case '?' :
                write(2, s, strlen(s));
                exit(1);
        }   
    }

    if(strcmp("{", argv[optind + 3]) != 0 || strcmp("}", argv[argc-1]) != 0){
        closedir(dir);
        write(2, errorMessage, strlen(errorMessage));
    }

    struct dirent *entry;
    int result = 0;
    while((entry = readdir(dir)) != NULL){
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || (a_option == 0 && entry->d_name[0] == '.')) continue;
        else{
            char **command = split_argv_for(argc - (optind + 3) - 2, argv+optind+4, entry->d_name, argv[3]);
            int retValue = applyCommand(command);
            if(retValue > result) result = retValue;
        }
    }

    exit(result);
}