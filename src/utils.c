#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>

#include "../include/structures/commandList.h"

void free_double_buffer(char **buffer, int bufsize);

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
char **split_command(char *command){
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
                    free_double_buffer(buffer, count);
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
    
    buffer[count] = (char *) NULL;
    return buffer;
}

/**
 * Fonction permettant de compter le nombre d'élément d'une partition de chaîne de caractère.
 * @param argv Partition de chaine de caractère
 * @return Taille de la partition.
 */
int partSize(char **argv){
    if(*(argv) == NULL) return 0;
    int i = 0;
    while(*(argv+i) != (char *) NULL){
        i++;
    }
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
    res[0] = '\0';

    if(strcmp(dirname, ".") != 0 || strcmp(dirname, "./") != 0) strcat(res, dirname);

    if(res[strlen(res)-1] != '/' || (strlen(filename) != 0 && filename[0] != '/')) strcat(res, "/");
    

    strcat(res, filename);

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
    for(int i = 0; i < bufsize; i++){
        if(buffer[i] != NULL) free(buffer[i]);
    }
    free(buffer);
}


/**
 * Fonction permettant de vérifier qu'une chaîne de caractère est égale à une autre sur les n premier octets.
 * @param buf1 Chaîne de caractère pouvant être plus grand que buf2.
 * @param buf2 Chaîne de caractère qui sera comparé avec buf1.
 * @return 1 si buf2 est bien égale au début de buf1 et 0 sinon.
 */
int isEqualFirst(const char *str, const char *var) {
    while (*var && *str == *var) {
        str++;
        var++;
    }
    return *var == '\0';
}

/**
 * Fonction permettant de remplacer le nom d'une variable par un nom donné dans une chaîne de caractère.
 * @param arg Chaîne de caractère.
 * @param var La variable dans la chaîne de caractère.
 * @param name Le nom par lequel la variable sera remplacé.
 * @return La chaîne de caractère identique si aucune variable remplaçable est présente et sinon la nouvelle chaîne de caractère avec la variable remplacé.
 */
char *replace_var(char *arg, char *var, char *name){
    char *buf;
    int bufsize = 0;
    int i = 0;

    int len = strlen(arg);
    while(i < len){
        if(arg[i] == '$' && isEqualFirst(arg + i, var)){
            bufsize += strlen(name);
            i += strlen(var);
        }
        else{
            bufsize++;
            i++;
        }
    }

    if((buf = malloc(bufsize + 1)) == NULL){
        perror("Erreur lors de l'allocation mémoire");
        exit(1);
    }

    i = 0;
    int j = 0;
    while(i < len){
        if(arg[i] == '$' && isEqualFirst(arg + i, var)){
            strcpy(buf + j, name);
            j += strlen(name);
            i += strlen(var);
        }
        else buf[j++] = arg[i++];
    }

    buf[j] = '\0';
    return buf;
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
    if(strlen(arg) == 0 || (arg[0] == '.' && strchr(arg + 1, '.') == NULL)) return NULL;

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
        return NULL;
    }

    strcpy(res, (arg+(strlen(arg)-count)));

    return res;
}

/**
 * Fonction permettant de vérifier si des spérateurs dans une partition sont équilibrées.
 * @param commande La commande.
 * @param open_separator Le séparateur ouvrant.
 * @param close_separator Le séparateur fermant.
 * @return 1 si équilibrées, 0 sinon.
 */
int isSeparatorGood(command *commande, char *open_separator, char *close_separator){
    int size = commande->size;
    char **partition = commande->list;
    int separatorOpen = 0;

    for(int i = 0; i < size; i++){
        if(strcmp(partition[i], open_separator) == 0){
            separatorOpen++;
        }
        else if(strcmp(partition[i], close_separator) == 0){
            if(separatorOpen == 0){
                return 0;
            }
            separatorOpen--;
        }
    }

    return separatorOpen == 0;
}
/**
 * Méthode permettant de vérifier si la chaîne de caractère fourni ne représente pas une valeur interdites.
 * @param argv La chaine de caractère.
 * @return 1 si c'est le cas et 0 sinon.
 */
int isIllegalName(char *argv){
    return strcmp(argv, "{") == 0 || strcmp(argv, "}") == 0 || strcmp(argv, "<") == 0 || strcmp(argv, ">") == 0 || strcmp(argv, "|") == 0 || strcmp(argv, ";") == 0 || strcmp(argv, "$") == 0;
}

/**
 * Fonction permettant de retirer l'extension d'un fichier.
 * @param arg Le nom du fichier.
 * @return Le nom sans extension.
 */
char *remove_extension_from_file(const char *arg){
    if (arg == NULL || strlen(arg) == 0) 
        return NULL;

    const char *last_dot = strrchr(arg, '.');

    if (last_dot == NULL || last_dot == arg) 
        return strdup(arg);

    size_t base_length = last_dot - arg;

    char *res = malloc(base_length + 1);
    if (res == NULL) {
        perror("Erreur lors de l'allocation mémoire");
        return NULL;
    }

    strncpy(res, arg, base_length);
    res[base_length] = '\0';

    return res;
}

/**
 * Fonction permettant de concaténer deux chaînes de caractères
 * @param arg1 Première chaîne de caractère.
 * @param arg2 Seconde chaîne de caractère.
 * @return Une chaîne de caractère représentant la concaténation de arg1 et arg2 aveec un ssaut à la ligne.
 */
char *concat(char *arg1, char *arg2){
    char *buffer;
    size_t size = strlen(arg1) + strlen(arg2) + 2;
    if((buffer = malloc((size)*sizeof(char))) == NULL){
        perror("Erreur lors de l'allocation mémoire");
        exit(1);
    }

    buffer[0] = '\0';

    snprintf(buffer, size, "%s%s\n", arg1, arg2);

    return buffer;
}

/**
 * Fonction permettant de vérifier si une partition correspond à une commande structurée.
 * @param partition La partition
 * @return 1 si cela représente bien une partition et 0 si ce n'est pas une partition du tout et -1 si cela devait représenter une partition.
 */
int isStructuredCommand(command *command,char *separator){
    char *s = "Usage: CMD_1 [separator] CMD_2 [separator] ... [separator] CMD_N\n";
    int hasSemi = 0;
    int hasBrace = 0;
    int hasQuote = 0;
    int partsize = command->size;
    char **partition = command->list;

    if(strcmp(separator, partition[0]) == 0 || strcmp(separator, partition[partsize-1]) == 0){
        write(2, s, strlen(s));
        return -1;
    }

    for(int i = 1; i < partsize-1; i++){
        if(strcmp(partition[i], "{") == 0) hasBrace++;
        else if(strcmp(partition[i], "}") == 0) hasBrace--;
        else if(strstr(partition[i], "\"") != NULL){
            if(hasQuote == 0) hasQuote = 1;
            else hasQuote = 0;
        }
        else if (strcmp(partition[i], separator) == 0 && hasBrace == 0 && hasQuote == 0) return 1;
    }

    return 0;
}


/**
 * Fonction permettant de savoir le nombre de commande dans une commande structurée.
 * @param Partition représentant une commande structurée.
 * @return Le nombre de commande dans une commande structurée.
 */
int nbCommand(command *command){
    int res = 0;
    int hasBrace = 0;
    char **partition = command->list;
    int partsize = command->size;

    for(int i = 0; i < partsize; i++){
        if(strcmp(partition[i], "{") == 0) hasBrace = 1;
        else if(strcmp(partition[i], "}") == 0) hasBrace = 0;
        else if(strcmp(partition[i], ";") == 0 && hasBrace == 0) res++;
    }

    if(res == 0) return res;
    return res + 1;
}


/**
 * Fonction permettant de créer une sous-partition d'une partition donnée.
 * @param commande La partition de base.
 * @param first Position de début.
 * @param last Position de fin.
 * @return La sous-partition.
 */
command *makeSubPartition(command *commande, int first, int last) {
    if(first > last) return NULL;
    
    int subsize = last - first + 2;  // +1 pour les indices, +1 pour le NULL terminal
    char **sub = malloc(subsize * sizeof(char *));
    if(!sub) {
        perror("Erreur d'allocation");
        exit(1);
    }
    
    for(int i = 0; i < subsize - 1; i++) {
        sub[i] = strdup(commande->list[first + i]);
        if(!sub[i]) {
            // Nettoyer en cas d'erreur
            for(int j = 0; j < i; j++) free(sub[j]);
            free(sub);
            perror("Erreur d'allocation");
            exit(1);
        }
    }
    
    sub[subsize - 1] = NULL;
    return newCommandList(sub);
}

/**
 * Fonction permettant de créer une liste de partitions pour une commande structurée.
 * @param commande La commande structurée.
 * @param size Pointeur pour stocker la taille de la liste.
 * @return La liste de partitions.
 */
command **makeStructuredCommand(command *commande, int *size, char *separator) {
    command **structuredCommand;
    char **partition = commande->list;
    int partSize = commande->size;
    
    int cmdCount = 1; 
    int hasBrace = 0;
    for(int i = 0; i < partSize; i++) {
        if(strcmp(partition[i], "{") == 0) hasBrace++;
        else if(strcmp(partition[i], "}") == 0) hasBrace--;
        else if(strcmp(partition[i], separator) == 0 && !hasBrace) cmdCount++;
    }
    
    if((structuredCommand = malloc(cmdCount * sizeof(command *))) == NULL) {
        perror("Erreur d'allocation");
        exit(1);
    }
    
    int cmdStart = 0;
    int cmdIndex = 0;
    hasBrace = 0;
    
    for(int i = 0; i < partSize; i++) {
        if(strcmp(partition[i], "{") == 0) hasBrace = 1;
        else if(strcmp(partition[i], "}") == 0) hasBrace = 0;
        else if(strcmp(partition[i], separator) == 0 && !hasBrace) {
            structuredCommand[cmdIndex] = makeSubPartition(commande, cmdStart, i - 1);
            cmdStart = i + 1;
            cmdIndex++;
        }
    }
    
    if(cmdStart < partSize) {
        structuredCommand[cmdIndex] = makeSubPartition(commande, cmdStart, partSize - 1);
        cmdIndex++;
    }
    
    *size = cmdIndex;
    return structuredCommand;
}



/**
 * Fonction permettant de savoir si un type de fichier correspond au bon caractère
 * @param filetype Le type de fichier
 * @param type Le caractère représetant le type de fichier (f pour fichier ordinaire, d pour les répertoires, l pour les liens symboliques, p pour les tubes).
 * @return 1 si cela correspond bien au type de fichier et 0 sinon
 */
int isSameType(mode_t mode, char f){
    switch(f){
        case 'f': return S_ISREG(mode);
        case 'd': return S_ISDIR(mode);
        case 'l': return S_ISLNK(mode);
        case 'p': return S_ISFIFO(mode);
        default: return false;
    }
}

/**
 * Fonction permettant de copier une partition de chaînes de caractères.
 * @param commande La partition à copier.
 * @return Une copie de la partition.
 */
command *copyPartition(command *commande){
    int size = commande->size;
    char **partition = commande->list;
    char **buf;

    if((buf = malloc((size+1) * sizeof(char *))) == NULL){
        perror("Erreur lors de l'allocation mémoire");
        exit(1);
    }

    for(int i = 0; i < size; i++){
        if((buf[i] = malloc(strlen(partition[i]) + 1)) == NULL){
            perror("Erreur lors de l'allocation mémoire");
            free_double_buffer(buf, i);
            exit(1);
        }

        strcpy(buf[i], partition[i]);
    }

    buf[size] = (char *) NULL;

    command *res = newCommandList(buf);

    return res;
}

/**
 * Fonction qui stocke la position d'un "else" das une commande structurée "if" s'il existe.
 * @param commande La commande structurée.
 * @param else_pos pointeur vers une variable de type int.
 */
void hasElse(char **commande, int *else_pos) {
    int i = 0;
    while (commande[i] != NULL) {
        if (strcmp(commande[i], "else") == 0) {
            *else_pos = i;
            break;
        }
        i++;
    }
}
