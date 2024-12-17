#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include "commandList.h"

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

    //printf("%s\n", arg);

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

    //printf("%d\n", bufsize);

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
 * Fonction permettant d'avoir une nouvelle partition de chaîne de caractère en prennant en compte le fait de remplacer $F par le nom d'une entrée donné
 * @param bufSize Taille du tableau de la chaîne de caractère.
 * @param argv Tableau de chaîne de caractère initiale.
 * @param filename Nom de l'entrée.
 * @return Nouvelle partition de chaîne de caractère avec $F remplacé par la référence de l'entrée.
 */
command * split_argv_for(int bufSize, char **argv, int argvsize, char *ref, char *varname){
    char **partition = NULL;
    int i;
    if((partition = malloc((bufSize + 1) * sizeof(char *))) == NULL) goto error;
    for(i = 0; i < argvsize-2; i++){
        if(strcmp(varname, argv[i]) == 0){
            if((partition[i] = malloc(strlen(ref) + 1)) == NULL) goto error;
            strcpy(partition[i], ref);
        }
        
        else{
            char *newvar = replace_var(argv[i], varname, ref);
            if((partition[i] = malloc(strlen(newvar) + 1)) == NULL) goto error;
            strcpy(partition[i], newvar);
            free(newvar);
        };
    }
    partition[bufSize] = (char *) NULL;
    command *res = newCommandList(partition);
    return res;

    error:
        perror("Erreur lors de la copie d'une chaine de caractère");
        if(partition != NULL) free_double_buffer(partition, bufSize+1);
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

int isBraceGood(char **partition){
    int size = partSize(partition);
    int braceOpen = 0;

    for(int i = 0; i < size; i++){
        if(strcmp(partition[i], "{") == 0) braceOpen++;
        else if(strcmp(partition[i], "}") == 0){
            if(braceOpen == 0) return 0;
            braceOpen--;
        }
    }

    return braceOpen == 0;
}

int isBraceGood2(char **partition){
    int size = partSize(partition);
    int braceOpen = 0;

    for(int i = 0; i < size; i++){
        if(strcmp(partition[i], "[") == 0) braceOpen++;
        else if(strcmp(partition[i], "]") == 0){
            if(braceOpen == 0) return 0;
            braceOpen--;
        }
    }

    return braceOpen == 0;
}

int isIllegalName(char *argv){
    return strcmp(argv, "{") == 0 || strcmp(argv, "}") == 0 || strcmp(argv, "<") == 0 || strcmp(argv, ">") == 0 || strcmp(argv, "|") == 0 || strcmp(argv, ";") == 0 || strcmp(argv, "$") == 0;
}

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
int isStructuredCommand(command *command){
    char *s = "Usage: CMD_1 ; CMD_2 ; ... ; CMD_N\n";
    int hasSemi = 0;
    int hasBrace = 0;
    int hasQuote = 0;
    int partsize = command->size;
    char **partition = command->list;

    if(strcmp(";", partition[0]) == 0 || strcmp(";", partition[partsize-1]) == 0){
        write(2, s, strlen(s));
        return -1;
    }

    for(int i = 1; i < partsize-1; i++){
        if(strcmp(partition[i], "{") == 0) hasBrace = 1;
        else if(strcmp(partition[i], "}") == 0) hasBrace = 0;
        else if(strstr(partition[i], "\"") != NULL){
            if(hasQuote == 0) hasQuote = 1;
            else hasQuote = 0;
        }
        else if (strcmp(partition[i], ";") == 0 && hasBrace == 0 && hasQuote == 0) return 1;
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
 * Fonction permettant de créer une sous-partition d'une partition donné en fonction de la position du début et de la fin.
 * @param command La partition de base
 * @param first Position du début de la sous-partition
 * @param last Position de la fin de la sous-partition
 * @return La nouvelle sous-partition voulue.
 */
command *makeSubPartition(command *commande, int first, int last){
    char **sub;
    char **partition = commande->list;
    int subsize = last-first+2;
    if((sub = malloc((subsize) * sizeof(char *))) == NULL){
        perror("Erreur lors de l'allocation mémoire");
        free_double_buffer(sub, subsize);
        exit(1);
    }

    int pos = 0;
    for(int i = first; i <= last; i++){

        int size = strlen(partition[i]) + 1;
        if((sub[pos] = malloc(size + 1)) == NULL){
            perror("Erreur lors de l'allocation mémoire");
            free_double_buffer(sub, i);
            exit(1);
        }

        strcpy(sub[pos], partition[i]);
        pos++;
    }

    sub[subsize-1] = (char *) NULL;
   
    command* res = newCommandList(sub);
    return res;
}

/**
 * Fonction permettant de créer une liste de partition dans le cas d'une commande structurée :
 * @param partition Partition représentant, normalement, une commande structurée.
 * @param size Pointeur qui correspondra à la taille de la liste de partition
 * @return La liste de partition de notre commande structurée.
 */
command **makeStructuredCommand(command *commande, int *size){
    command **structuredCommand;
    char **partition = commande->list;
    int scsize = nbCommand(commande);
    int partSize = commande->size;

    int first = 0;
    int last = 0;
    int pos = 0;

    int hasBrace = 0;

    if((structuredCommand = malloc(scsize * sizeof(command *))) == NULL){
        perror("Erreur lors de l'allocation mémoire");
        exit(1);
    }

    for(int i = 0; i < partSize; i++){
        if(strcmp(partition[i], "{") == 0) hasBrace = 1;
        else if(strcmp(partition[i], "}") == 0) hasBrace = 0;
        else if(strcmp(partition[i], ";") == 0 && hasBrace == 0){
            last = i-1;
            structuredCommand[pos] = makeSubPartition(commande, first, last);
            first = i+1;
            pos++;
        }
    }

    if(first < partSize) {
        structuredCommand[pos] = makeSubPartition(commande, first, partSize-1);
        pos++;
    }
    *size = pos;

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
 * Fonction permettant de copier une partition de chaîne de caractère.
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
 * Fonction qui verifie si il y a un "else" dans une commande structurée if...
 * @param command une commande
 * @return la indice/position du "else" ou -1 si il n'y a pas de "else" dans la commande*/
int hasElse(char **command) {
    int i = 0;
    while (command[i] != NULL) {
        if (strcmp(command[i], "else") == 0) {
            return i;
        }
        i++;
    }

    return -1;
}


/**
 * Fonction qui exécute le test correspondant.
 * @param cmd_test un test/une condition dans une commande structurée if...
 * @return la valeur de retour du test (echec ou réussite) ou -1 si le test n'est pas reconnu '*/
int applyTest(char **cmd_test) {
    if (strcmp(cmd_test[0], "-d") == 0) {
        DIR *dir = opendir(cmd_test[1]);
        int r = (access(cmd_test[1], F_OK) == 0 && dir != NULL) ? 0 : 1;
        closedir(dir);
        return r;
    }
    if (strcmp(cmd_test[0], "-f") == 0) {
        struct stat file_stat;
        if (access(cmd_test[1], F_OK) == 0 && stat(cmd_test[1], &file_stat) == 0) {
            return S_ISREG(file_stat.st_mode) ? 0 : 1;
        }
        return 1;
    }
    if (strcmp(cmd_test[0], "-e") == 0) {
        return (access(cmd_test[1], F_OK) == 0) ? 0 : 1;
    }
    if (strcmp(cmd_test[0], "-r") == 0) {
        return (access(cmd_test[1], R_OK) == 0) ? 0 : 1;
    }
    if (strcmp(cmd_test[0], "-w") == 0) {
        return (access(cmd_test[1], W_OK) == 0) ? 0 : 1;
    }
    if (strcmp(cmd_test[0], "-x") == 0) {
        return (access(cmd_test[1], X_OK) == 0) ? 0 : 1;
    }
    if (strcmp(cmd_test[0], "-L") == 0) {
        struct stat file_stat;
        if (lstat(cmd_test[1], &file_stat) == 0) {
            return S_ISLNK(file_stat.st_mode) ? 0 : 1;
        }
        return 1;
    }
    if (strcmp(cmd_test[1], "-eq") == 0) {
        int a = atoi(cmd_test[0]);
        int b = atoi(cmd_test[2]);
        return (a == b) ? 0 : 1;
    }
    if (strcmp(cmd_test[1], "=") == 0) {
        return (strcmp(cmd_test[0], cmd_test[2]) == 0) ? 0 : 1;
    }
    if (strcmp(cmd_test[1], "!=") == 0) {
        return (strcmp(cmd_test[0], cmd_test[2]) != 0) ? 0 : 1;
    }
    if (strcmp(cmd_test[0], "-z") == 0) {
        return (strlen(cmd_test[1]) == 0) ? 0 : 1;
    }
    if (strcmp(cmd_test[0], "-n") == 0) {
        return (strlen(cmd_test[1]) > 0) ? 0 : 1;
    }
    if (strcmp(cmd_test[0], "!") == 0) {
        int sub_test_result = applyTest(&cmd_test[1]);
        return (sub_test_result == 0) ? 1 : 0;
    }
    if (strcmp(cmd_test[1], "-o") == 0) {
        int sub_test_result1 = applyTest(&cmd_test[0]);
        int sub_test_result2 = applyTest(&cmd_test[2]);
        return (sub_test_result1 == 0 || sub_test_result2 == 0) ? 0 : 1;
    }
    if (strcmp(cmd_test[1], "-a") == 0) {
        int sub_test_result1 = applyTest(&cmd_test[0]);
        int sub_test_result2 = applyTest(&cmd_test[2]);
        return (sub_test_result1 == 0 && sub_test_result2 == 0) ? 0 : 1;
    }

    // Test non reconnu
    return -1;
}