#ifndef CMD_CD_H
#define CMD_CD_H

#include "../command.h"

/**
 * Méthode permettant de changer la position courant de l'arborescence.
 * @param commande La commande ayant appelé la méthode.
 * @return 0 si le déplacement s'est bien deroulé et 1 sinon.
 */
int cmd_cd(command *commande);

#endif