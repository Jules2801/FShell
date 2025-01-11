#ifndef CMD_CD_FOR
#define CMD_CD_FOR

#include "../command.h"
#include "../../include/structures/commandList.h"

/**
 * Méthode permettant d'appliquer la commande for.
 * @param commande La commande ayant appelé la méthode et contenant les arguments nécessaires si correct.
 * @param commandeList Pointeur vers une liste chaînée où sera stocké toutes les commandes à exécuter en fonction des fichiers.
 * @param parralel Valeur permettant de savoir si une approche parallèle est nécessaire. Elle vaut -1 si ce n'est pas le cas et une valeur strictement positif sinon.
 * @return 0 si la méthode s'est bien deroulé, 1 si la méthode s'est mal déroulé et 2 en cas d'erreur de syntaxe.
 */
int cmd_for(command *commande, linkedList *commandList, int *parralel);

#endif