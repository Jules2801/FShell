#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>


/**
 * Fonction permettant de compter le nombre de mots dans une chaîne de caractère.
 * @param command Une chaine de caractêre.
 * @return Un entier représentant le nombre de mots dans la chaîne de caractère.
 */
int count_words(char *command){
    int count = 0;
    bool inWord = false;

    for(int i = 0; i < strlen(command); i++){
        if(isspace(*(command+i))) inWord = false;
        else{
            if(!inWord){
                count++;
                inWord = true;
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
char ** split_command(char *command){
    char **buffer;
    int length = (count_words(command)+1)*sizeof(char *);

    if((buffer = malloc(length)) == NULL){
        perror("Erreur lors de l'allocation mémoire");
        exit(1);
    }

    int count = 0;
    bool inWord = false;
    int start = 0;

    for(int i = 0; i <= strlen(command); i++){
        if(isspace(*(command+i)) || *(command+i) == '\0'){
            if(inWord){
                int wordLen = i - start;
                buffer[count] = malloc((wordLen + 1) * sizeof(char));
                if (buffer[count] == NULL) {
                    perror("Erreur lors de l'allocation mémoire");
                    return NULL;
                }
                strncpy(buffer[count], command + start, wordLen);
                buffer[count][wordLen] = '\0';
                count++;
                inWord = false;
            }
        }
        else {
            if(!inWord){
                start = i;
                inWord = true;
            }
        }
    }
    
    *(buffer+length-1) = NULL;
    return buffer;
}

/**
 * Fonction permettant d'exécuter une commande externe (si c'est le cas) donné en argument.
 * @param command Le nom de la commande externe
 * @return -1 si une erreur s'est produite avant recouvrement, 1 si une erreur s'est produite pendant le recouvrement et 0 si tout s'est bien passé.
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
        if(WIFEXITED(status) && WEXITSTATUS(status) == 0) return 0;
        return 1;
    }
}

int main(int argc, char **argv){
    int errorCount = 0;
    char *input;

    while(1) {
        input = readline("./fsh : ");

        if(input == NULL) break;
        if(*input) add_history(input);
        
        char **command = split_command(input);

        //TODO : Gérer ici le cas des commandes internes

        //Lorsqu'aucune commande interne n'a été exécuté, essayer les commandes externes qui est fait avec le code suivant :
        int val = commande_externe(command);
        if(val == 1) errorCount++;
        if(val == -1) {
            perror("Erreur lors du recouvrement");
            exit(1);
        } 
    }
}