#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>


#define  MAX_BUFFER_SIZE  1024;

char *prompt(int value){ //TODO: vérifié si il est bien nécessaire de connaite le boolean en plus de la valeure

    char *color;
    int prompt_status_len; //Représente la taille de [SIG] ou [(0 ou 1)] 5 pour le premier 3 pour le 2em
    int numDigits = snprintf(NULL, 0, "%d", value);
    

    if(value < 0){ // On vérifie si un signal a interompue la commande précédente 
        color = "\033[36m"; // on change la couleur en conséquence 
        prompt_status_len = 5;
    }
    else{ //Sinon on vérifie si la commande précédente donnait une erreur ou non
        prompt_status_len = 2 + numDigits;
        if (value > 0){
        color = "\033[91m";
        }
        else{
        color = "\033[32m";
        }
    }
 
    
char *path = getcwd(NULL, 0); // Pour avoir le chemin total
    char fullPath[41]; // +10 pour les deux balises de couleurs de taille 5 (+1 pour \0)

    int sizePath = strlen(path);

    if(sizePath>(30-prompt_status_len-2)){ // On retire 2 car on compte "$ "
        
        int trunc =  (sizePath-(30-prompt_status_len-2-3)); // Est l'index a partir du quelle on va tronquer le chemin -3 car on compte "..."
        memmove(path +3 ,path + trunc,30-prompt_status_len-2-3+1);// on décale de 3 pour ajouter "..."
        path[0] = '.';
        path[1] = '.';
        path[2] = '.';
        
    }
    if(value < 0) snprintf(fullPath, sizeof(fullPath),"%s[%s]%s$ \033[00m",color,"SIG",path);
    else snprintf(fullPath, sizeof(fullPath),"%s[%d]%s$ \033[00m",color,value,path);

    free(path);

    char *result = malloc(strlen(fullPath) + 1); 
    if (result) {
        strcpy(result, fullPath);
    }
    return result;  
}


    
