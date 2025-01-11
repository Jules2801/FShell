#ifndef CMD_EXIT_H
#define CMD_EXIT_H

#include "../command.h"

/**
 * Méthode permettant de sortir du programme en fonction de la valeur de error.
 * @param commande La commande ayant permis d'appeler la méthode exit.
 * @param error La valeur de la sortie si la taille de commande vaut 1.
 * @return 1 si la sortie du programme n'a pas fonctionné.
 */
int cmd_exit(command *commande, int *error);

#endif
