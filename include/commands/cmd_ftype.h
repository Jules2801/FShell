#ifndef CMD_CD_FTYPE
#define CMD_CD_FTYPE

#include "../command.h"

/**
 * Méthode permettant de savoir le type d'un fichier donné.
 * @param commande La commande ayant appelé la méthode et contenant la référence du fichier si correcte.
 * @return 0 si la méthode s'est bien deroulé et 1 sinon.
 */
int cmd_ftype(command *commande);

#endif