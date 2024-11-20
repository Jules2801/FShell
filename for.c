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
    int result = 0;
    char *errorMessage = "usage: for [variable-name] in REP [-A] [-r] [-e EXT] [-t TYPE] [-p MAX] { CMD }\n";

    DIR *dir = NULL;
    char *varname = NULL;

    int a_option = 0;
    int r_option = 0;
    char *extension = NULL;
    char type;
    int max_parralel = -1;

    char *s = "Erreur : option inconnue ou argument manquant\n";

    struct dirent *entry = NULL;

    if(argc < 7 || strcmp("in", argv[2])){
        write(2, errorMessage, strlen(errorMessage));
        exit(1);
    }

    if((dir = opendir(argv[3])) == NULL){
        perror("Erreur lors de l'ouverture du fichier");
        result = 1;
        goto cleanup_return;
    }

    if((varname = malloc((strlen(argv[1]) + 2) * sizeof(char))) == NULL){
        perror("Erreur lors de l'allocation mémoire");
        result = 1;
        goto cleanup_return;
    }
    if(strcat(varname, "$") == NULL){
        perror("Erreur lors de la concaténation d'une chaîne de caractère");
        result = 1;
        goto cleanup_return;
    }
    if(strcat(varname, argv[1]) == NULL){
        perror("Erreur lors de la concaténation d'une chaîne de caractère");
        result = 1;
        goto cleanup_return;
    }

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
                    result = 1;
                    goto cleanup_return;
                }

                strcpy(extension, optarg);
                *(extension + strlen(optarg)) = '\0';

                break;

            case 't' :
                if(strlen(optarg) != 1){
                    char *s = "Vous devez fournir un caractère\n";
                    write(STDERR_FILENO, s, strlen(s));
                    result = 1;
                    goto cleanup_return;
                }

                type = *(optarg);

                break;
            
            case 'p' :
                // TODO : Créer une fonction permettant de vérifier qu'on entre bien un entier
                max_parralel = atoi(optarg);

                break;

            case '?' :
                write(2, s, strlen(s));
                result = 1;
                goto cleanup_return;
        }   
    }
    int firstind = optind + 3;
    if(strcmp("{", argv[firstind]) != 0 || strcmp("}", argv[argc-1]) != 0){
        write(2, errorMessage, strlen(errorMessage));
        result = 1;
        goto cleanup_return;
    }

    while((entry = readdir(dir)) != NULL){
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || (a_option == 0 && entry->d_name[0] == '.')) continue;
        else{
            int bufsize = argc - (firstind) - 2;
            char **command = split_argv_for(argc - firstind - 2, argv + firstind + 1, argc - firstind, argv[3], entry->d_name, varname);
            int retValue = applyCommand(command);
            if(retValue > result) result = retValue;
        }
    }

    goto cleanup_return;

    cleanup_return :
        if(dir != NULL) closedir(dir);
        if(varname != NULL) free(varname);
        if(extension != NULL) free(extension);
        if(entry != NULL) free(entry);

    exit(result);
}