#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <dirent.h>

int intern_for(int argc, char **argv);

/**
 * Fonction permettant de compter le nombre de mots dans une chaîne de caractère.
 * @param command Une chaine de caractêre.
 * @return Un entier représentant le nombre de mots dans la chaîne de caractère.
 */
int count_words(char *command){
    int count = 0;
    int inWord = 0;

    for(int i = 0; i < strlen(command); i++){
        if(isspace(*(command+i))) inWord = 0;
        else{
            if(!inWord){
                count++;
                inWord = 1;
            }
        }
    }

    return count;
}

/**
 * Fonction permettant de couper une chaîne de caractère en un tableau de chaîne de caractère avec NULL en dernière position du tableau.
 * @param command Une chaîne de caractère.
 * @return Le tableau de chaîne de caractère.
 */
char **split_command(char *command, int *size){
    char **buffer;
    int length = (count_words(command)+1)*sizeof(char *);

    if((buffer = malloc(length)) == NULL){
        perror("Erreur lors de l'allocation mémoire");
        exit(1);
    }

    int count = 0;
    int inWord = 0;
    int start = 0;

    for(int i = 0; i <= strlen(command); i++){
        if(isspace(*(command+i)) || *(command+i) == '\0'){
            if(inWord){
                int wordLen = i - start;
                buffer[count] = malloc((wordLen + 1) * sizeof(char));
                if (buffer[count] == NULL) {
                    perror("Erreur lors de l'allocation mémoire");
                    exit(1);
                }
                strncpy(buffer[count], command + start, wordLen);
                buffer[count][wordLen] = '\0';
                count++;
                inWord = 0;
            }
        }
        else {
            if(!inWord){
                start = i;
                inWord = 1;
            }
        }
    }
    
    *(buffer+length-1) = (char *) NULL;
    return buffer;
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

/**
 * Fonction permettant d'appliquer la commande for.
 * @param command Tout ce qui sera utiles pour appliquer la commande externe, telle que son nom et ses options.
 * @return Un entier supérieure à 1 si une erreur s'est produite pendant le recouvrement et 0 si tout s'est bien passé.
 */
int forCommand(char **command, int size){
    pid_t pid = 0;
    if((pid = fork()) == 0){
        exit(intern_for(size, command));
    }
    else{
        int status;
        waitpid(pid, &status, 0);
        if(WIFEXITED(status)) return WEXITSTATUS(status);
        return 1;
    }
}

/**
 * Fonction permettant de compter le nombre d'élément d'une partition de chaîne de caractère.
 * @param argv Partition de chaine de caractère
 * @return Taille de la partition.
 */
int partSize(char **argv){
    int i = 0;
    while(*(argv+i) != (char *) NULL) i++;
    return i;
}

/**
 * Fonction permettant d'appliquer toutes les fonctions internes/externes.
 * @param command Tout ce qui sera utiles pour appliquer la commande externe, telle que son nom et ses options.
 * @return Un entier supérieure à 1 si une erreur s'est produite pendant le recouvrement et 0 si tout s'est bien passé.
 */
int applyCommand(char **command){
    if(strcmp(command[0], "for") == 0){
        return forCommand(command, partSize(command));
    }
    else return commande_externe(command);
}

/**
 * Fonction permettant d'avoir l'emplacement relative d'une entrée depuis un répertoire donné.
 * @param dirname La référence relative du répertoir actuelle.
 * @param filename Le nom de l'entrée.
 * @return Une chaîne de caractère représentant l'emplacement relative d'une entrée.
 */
char * getRelativeReference(char *dirname, char *filename){
    char *res = NULL;
    if((res = malloc(strlen(dirname) + strlen(filename) + 2)) == NULL) goto error;

    if(strcpy(res, dirname) == NULL) goto error;

    if(res[strlen(res)-1] != '/') if(strcat(res, "/") == NULL) goto error;
    

    if(strcat(res, filename) == NULL) goto error;

    return res;
    
    error:
        perror("Erreur lors de la copie d'une chaîne de caractère");
        exit(1);
}

/**
 * Fonction permettant de libérer un buffer représentant un tableau de chaîne de caractère.
 * @param buffer Le tableau de chaine de caractère
 * @param bufsize La taille du buffer
 */
void free_double_buffer(char **buffer, int bufsize){
    puts("je suis ici");
    for(int i = 0; i < bufsize; i++){
        puts("c'est a cause de ça");
        if(*(buffer) + i != NULL) free(*(buffer)+ i);
    }
    free(buffer);
}

/**
 * Fonction permettant d'avoir une nouvelle partition de chaîne de caractère en prennant en compte le fait de remplacer $F par le nom d'une entrée donné
 * @param bufSize Taille du tableau de la chaîne de caractère.
 * @param argv Tableau de chaîne de caractère initiale.
 * @param filename Nom de l'entrée.
 * @param dirname Reférence du répertoire courant.
 * @return Nouvelle partition de chaîne de caractère avec $F remplacé par la référence de l'entrée.
 */
char ** split_argv_for(int bufSize, char **argv, int argvsize, char *dirname, char *filename, char *varname){
    char **res = NULL;
    int i;
    if((res = malloc((bufSize + 1) * sizeof(char *))) == NULL) goto error;
    for(i = 0; i < argvsize-1; i++){
        if(strcmp(varname, argv[i]) == 0){
            res[i] = getRelativeReference(dirname, filename);
        }
        
        else{
            if((res[i] = malloc(strlen(argv[i] + 1))) == NULL) goto error;
            if(strcpy(res[i], argv[i]) == NULL) goto error;
        };
    }
    res[bufSize] = (char *) NULL;
    return res;

    error:
        perror("Erreur lors de la copie d'une chaine de caractère");
        if(res != NULL) free_double_buffer(res, bufSize+1);
        exit(1);
}

int intern_for(int argc, char **argv){
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

    return result;
}