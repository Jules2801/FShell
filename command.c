#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <dirent.h>
#include "utils.h"
#include <sys/stat.h>

int commande_externe(char **command);

int forCommand(char **command);
int cmd_for(int argc, char **argv);

int exitCommand(char **command, int *error);
int cmd_exit(char **command, int *error);


int ftypeCommand(char **command);
int cmd_ftype(char **command);

int pwdCommand(char **command);
int cmd_pwd(char **command);

/**
 * Fonction permettant d'appliquer toutes les fonctions internes/externes.
 * @param command Tout ce qui sera utiles pour appliquer la commande externe, telle que son nom et ses options.
 */
void applyCommand(char **command, int *error){
    if(strcmp(command[0], "pwd") == 0) *error = pwdCommand(command);
    else if(strcmp(command[0], "exit") == 0) *error = exitCommand(command, error);
    else if(strcmp(command[0], "ftype") == 0) *error = ftypeCommand(command);
    else if(strcmp(command[0], "for") == 0) *error =  forCommand(command);
    else *error = commande_externe(command);
}

/**
 * Fonction permettant d'appliquer la commande for.
 * @param command Tout ce qui sera utiles pour appliquer la commande externe, telle que son nom et ses options.
 * @param size Taille de la partition de chaîne de caractère.
 * @return Un entier supérieure à 1 si une erreur s'est produite pendant le processus et 0 si tout s'est bien passé.
 */
int forCommand(char **command){
    pid_t pid = 0;
    if((pid = fork()) == 0){
        exit(cmd_for(partSize(command), command));
    }
    else{
        int status;
        waitpid(pid, &status, 0);
        if(WIFEXITED(status)) return WEXITSTATUS(status);
        return 1;
    }
}

/**
 * Fonction permettant d'appliquer la commande pwd.
 * @return Un entier supérieure à 1 si une erreur s'est produite pendant le processus et 0 si tout s'est bien passé.
 */
int pwdCommand(char **command){
    pid_t pid = 0;
    if((pid = fork()) == 0){
        exit(cmd_pwd(command));
    }
    else{
        int status;
        waitpid(pid, &status, 0);
        if(WIFEXITED(status)) return WEXITSTATUS(status);
        return 1;
    }
}

/**
 * Fonction permettant d'appliquer la commande exit.
 * @return Un entier supérieure à 1 si une erreur s'est produite pendant le processus et 0 si tout s'est bien passé.
 */
int exitCommand(char **command, int *error){
    return cmd_exit(command, error);
}

/**
 * Fonction permettant d'appliquer la commande exit.
 * @return Un entier supérieure à 1 si une erreur s'est produite pendant le processus et 0 si tout s'est bien passé.
 */
int ftypeCommand(char **command){
    pid_t pid = 0;
    if((pid = fork()) == 0){
        exit(cmd_ftype(command));
    }
    else{
        int status;
        waitpid(pid, &status, 0);
        if(WIFEXITED(status)) return WEXITSTATUS(status);
        return 1;
    }
}

/**
 * Fonction permettant d'exécuter une commande externe (si c'est le cas) donné en argument.
 * @param command Tout ce qui sera utiles pour appliquer la commande externe, telle que son nom et ses options.
 * @return Un entier supérieure à 1 si une erreur s'est produite pendant le recouvrement et 0 si tout s'est bien passé.
 */
int commande_externe(char **command){
    pid_t pid = fork();
    if(pid == -1){
        perror("Erreur lors de la création du processus");
        return -1;
    }

    if(pid == 0){
        execvp(command[0], command);
        exit(1);
    }

    else{
        int status;
        waitpid(pid, &status, 0);
        if(WIFEXITED(status)) return WEXITSTATUS(status);
        return 1;
    }
}


int cmd_exit(char  **command, int *error) {
    char *errorMessage = "usage: exit [VAL]\n";
    int size = partSize(command);

    if(size == 0){
        write(2, errorMessage, strlen(errorMessage));
        return 1;
    }
    else if(size == 1) exit(*error);
    else{
        int err = stringToPositiveInteger(command[1]);
        if(err == -1){
            write(2, errorMessage, strlen(errorMessage));
            return 1;
        }
        else exit(err);
    }
}

int cmd_ftype(char **command) {
    char *errorMessage = "usage: ftype REF\n";
    int size = partSize(command);

    if(size != 2){
        write(2, errorMessage, strlen(errorMessage));
        return 1;
    }

    struct stat statbuf;

    if (stat(command[1], &statbuf) == -1) {
        perror("stat");
        return 1;
    }
    char *s;
    if (S_ISDIR(statbuf.st_mode)) {
        s = "directory\n";
    } else if (S_ISREG(statbuf.st_mode)) {
        s = "regular file\n";
    } else if (S_ISLNK(statbuf.st_mode)) {
        s = "symbolic link\n";
    } else if (S_ISFIFO(statbuf.st_mode)) {
        s = "named pipe\n";
    } else {
        s = "other\n";
    }
    write(1, s, strlen(s));
    return 0;
}

int cmd_pwd(char **command) {
    char *errorMessage = "pwd: Too many argument!\n";
    int size = partSize(command);

    if(size != 1){
        write(2, errorMessage, strlen(errorMessage));
        return 1;
    }
    char cwd[1024];
    
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        write(1, cwd, strlen(cwd));
        return 0;
    } else {
        perror("getcwd");
        return 1;
    }
}

int cmd_for(int argc, char **argv){
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
        return 1;
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
                if((max_parralel = stringToPositiveInteger(optarg)) == -1){
                    write(2, errorMessage, strlen(errorMessage));
                    result = 1;
                    goto cleanup_return;
                };

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
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || (a_option == 0 && entry->d_name[0] == '.') || (extension != NULL && strcmp(extension, extract_extension(entry->d_name)))) continue;
        else{
            int bufsize = argc - (firstind) - 2;
            char **command = split_argv_for(argc - firstind - 2, argv + firstind + 1, argc - firstind, argv[3], entry->d_name, varname);
            applyCommand(command, &result);
        }
    }

    goto cleanup_return;

    cleanup_return :
        if(dir != NULL) closedir(dir);
        if(varname != NULL) free(varname);
        if(extension != NULL) free(extension);
        if(entry != NULL) free(entry);

    return result;
}