#include "../../include/command.h"
#include "../../include/utils.h"

/**
 * Méthode permettant de sortir du programme en fonction de la valeur de error.
 * @param commande La commande ayant permis d'appeler la méthode exit.
 * @param error La valeur de la sortie si la taille de commande vaut 1.
 * @return 1 si la sortie du programme n'a pas fonctionné.
 */
int cmd_exit(command *commande, int *error) {
    char *errorMessage = "usage: exit [VAL]\n";
    int size = commande->size;

    if(size == 0){
        write(2, errorMessage, strlen(errorMessage));
        return 1;
    }
    else if(size == 1){
        if(*error < 0) exit(255);
        else exit(*error);
    }
    else if(size == 2){
        int err = stringToPositiveInteger(commande->list[1]);
        if(err == -1){
            write(2, errorMessage, strlen(errorMessage));
            return 1;
        }
        else exit(err);
    }
    else{
        char *s = "exit: trop d'arguments.\n";
        write(2, s, strlen(s));
        return 1;
    }
}