#ifndef COMMAND_H
#define COMMAND_H
#include "../include/structures/commandList.h"

/**
 * Fonction permettant d'appliquer toutes les commandes internes/externes.
 * @param command Tout ce qui sera utiles pour appliquer la commande, telle que son nom et ses options.
 * @param error Un pointeur d'entier où sera stocké la valeur de retour de la commande fourni.
 */
void applyCommand(command *commande, int *error);

#endif