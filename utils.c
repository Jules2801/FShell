#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <dirent.h>

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

/**
 * Fonction permettant de vérifier qu'une chaîne de caractère représente bien un entier positif.
 * @param arg La chaîne de caractère.
 * @return 1 si cela représente bien un entier ou 0 sinon.
 */
int isPositiveInteger(char *arg){
    for(int i = 0; i < strlen(arg); i++){
        if(!(arg[i] >= '0' && arg[i] <= '9')) return 0;
    }
    return 1;
}

/**
 * Fonction permettant de convertir une chaîne de caractère en entier positif.
 * @param arg La chaîne de caractère.
 * @return L'entier représenté par la chaîne de caractère et -1 sinon.
 */
int stringToPositiveInteger(char *arg){
    if(!isPositiveInteger(arg) || arg == NULL) return -1;

    int res = 0;
    for(int i = 0; i < strlen(arg); i++){
        res = res * 10 + (arg[i] - '0');
    }

    return res;
}

/**
 * Fonction permettant d'extraite l'extension d'une référence donné.
 * @param arg Référence contenant une extension ou non.
 * @return L'extension si le fichier possède une extension et NULL sinon.
 */
char *extract_extension(char* arg){
    if(strlen(arg) == 0 || arg[0] == '.' && strchr(arg + 1, '.') == NULL) return NULL;

    int count = 0;
    int found_dot = 0;
    for(int i = strlen(arg)-1; i > 0; i--){
        if(arg[i] == '.'){
            found_dot = 1;
            break;
        }
        count++;
    }

    if(!found_dot) return NULL;

    char *res;
    if((res = malloc((count + 1) * sizeof(char))) == NULL){
        perror("Erreur lors de l'allocation mémoire");
        exit(1);
    }

    strcpy(res, (arg+(strlen(arg)-count)));

    return res;
}