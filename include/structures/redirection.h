#ifndef REDIRECTION_H
#define REDIRECTION_H

#include "commandList.h"

/**
 * Enum permettant de différencier tous les cas de redirections possibles.
 */
typedef enum {
    RSTDIN,
    RSTDOUT,
    RSTDOUT_WITH_CRUSHING,
    RSTDOUT_WITH_APPEND,
    RSTDERR,
    RSTDERR_WITH_CRUSHING,
    RSTDERR_WITH_APPEND
} Redtype;

/**
 * Structure permettant d'avoir des informations sur le type de redirection et sur le fichier correspondant.
 */
typedef struct {
    Redtype type;
    char *filename;
} Redirection;

/**
 * Méthode permettant de compter le nombre de redirection vers l'entrée standard.
 * @param commande La commande contenant les redirections ou non.
 */
int count_stdin(command *commande);

/**
 * Méthode permettant de compter le nombre de redirection vers la sortie standard.
 * @param commande La commande contenant les redirections ou non.
 */
int count_stdout(command *commande);

/**
 * Méthode permettant de compter le nombre de redirection vers la sortie erreur.
 * @param commande La commande contenant les redirections ou non.
 */
int count_stderr(command *commande);

/**
 * Méthode permettant libérer la liste des redirections en fonction d'une taille donné.
 * @param redirections La liste des redirections.
 * @param size La limite du tableau jusqu'où la libération de mémoire sera faite.
 */
void free_redirection_list(Redirection **redirections, int size);

/**
 * Méthode permettant de libérer la mémoire d'une structure redirection et son pointeur.
 * @param redirection Le pointeur vers la structure redirection.
 */
void free_redirection(Redirection *redirection);

/**
 * Méthode permettant d'avoir le tableau de toutes les redirections vers l'entrée standard dans la commande.
 * @param commande La commande contenant les redirections ou non.
 * @param size Un pointeur vers où sera stocké la taille du tableau finale.
 * @return Le tableau de toutes les redirections vers l'entrée standard.
 */
Redirection **get_all_stdin(command *commande, int *size);

/**
 * Méthode permettant d'avoir le tableau de toutes les redirections vers la sortie standard dans la commande.
 * @param commande La commande contenant les redirections ou non.
 * @param size Un pointeur vers où sera stocké la taille du tableau finale.
 * @return Le tableau de toutes les redirections vers la sortie standard.
 */
Redirection **get_all_stdout(command *commande, int *size);

/**
 * Méthode permettant d'avoir le tableau de toutes les redirections vers la sortie erreur dans la commande.
 * @param commande La commande contenant les redirections ou non.
 * @param size Un pointeur vers où sera stocké la taille du tableau finale.
 * @return Le tableau de toutes les redirections vers la sortie erreur.
 */
Redirection **get_all_stderr(command *commande, int *size);

/**
 * Méthode permettant de retirer toutes les redirections présente dans une ligne de commande.
 * @param commande La ligne de commande où sera retiré les redirections.
 * @return La nouvelle ligne de commande sans redirections.
 */
command *remove_all_redirection(command *commande);

/**
 * Méthode permettant de créer le descripteur du fichier dont la référence est contenu dans une structure Redirection.
 * @param red Pointeur vers la structure Redirection.
 * @return Le descripteurs du fichier correspondant à la redirection.
 */
int getFile(Redirection *red);

#endif