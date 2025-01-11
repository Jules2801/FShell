#ifndef CMD_CD_PWD
#define CMD_CD_PWD

#include "../command.h"

/**
 * Méthode permettant de recevoir la référence absolue en fonction de la position courante.
 * @param commande La commande ayant appelé la méthode.
 * @return 0 si la méthode s'est bien deroulé et 1 sinon.
 */
int cmd_pwd(command *commande);

#endif