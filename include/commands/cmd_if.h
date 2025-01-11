#ifndef CMD_CD_IF
#define CMD_CD_IF

#include "../command.h"

/**
 * Méthode permettant d'appliquer la commande if.
 * @param commande La commande ayant appelé la méthode et contenant les arguments nécessaires si correct.
 * @return 0 si la méthode s'est bien deroulé, 1 si la méthode s'est mal déroulé et 2 en cas d'erreur de syntaxe.
 */
int cmd_if(command *commande);

#endif