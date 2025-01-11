#include <stdio.h>
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h> 

#include "../../include/command.h"

/**
 * Méthode permettant de recevoir la référence absolue en fonction de la position courante.
 * @param commande La commande ayant appelé la méthode p∂.
 * @return 0 si la méthode s'est bien deroulé et 1 sinon.
 */
int cmd_pwd(command *commande) {

    char cwd[1024];

    if(commande->size != 1){
        char *s = "pwd: trop d'arguments\n";
        write(2, s, strlen(s));
        return 1;
    }

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        strcat(cwd, "\n");
        write(1, cwd, strlen(cwd));
        return 0;
    } else {
        perror("getcwd");
        return 1;
    }
}