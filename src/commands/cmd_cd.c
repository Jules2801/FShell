#include <stdio.h>
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h> 

#include "../../include/command.h"
#include "../../include/global.h"

/**
 * Fonction permettant de mettre a jour le repertoire courant et l'ancien
 * @param new_directory le chemin du nouveau repertoire courant
 */
void update_directories(const char *new_directory) {
    if(new_directory == NULL) {
        char *s = "Erreur: le nouveau répertoire est NULL.\n";
        write(2, s, strlen(s));
        return;
    }

    char *new_directory_copy = strdup(new_directory);
    if (new_directory_copy == NULL) {
        char *s = "Erreur: Échec de l'allocation mémoire pour le nouveau répertoire.\n";
        write(2, s, strlen(s));
        return;
    }

    if (previous_directory != NULL) {
        free(previous_directory);
    }

    previous_directory = current_directory;
    current_directory = new_directory_copy;
}

/**
 * Méthode permettant de changer la position courant de l'arborescence.
 * @param commande La commande ayant appelé la méthode cd.
 * @return 0 si le déplacement s'est bien deroulé et 1 sinon.
 */
int cmd_cd(command *commande){
    char *path;
    int partSize = commande->size;
    if(partSize>2){
        char *s = "cd: trop d'arguments\n";
        write(2, s, strlen(s));
        return 1;
    }
    if(partSize==1){
        char buffer[256];
        if(isInRed && (fgets(buffer, sizeof(buffer), stdin) != NULL)){
            buffer[strcspn(buffer, "\n")] = '\0';
            if (strlen(buffer) == 0) {
                write(1, "Chemin non valide ou vide\n", 26);
                return 1;
            }
            path = buffer;
        }
        else{
            path = getenv("HOME");
            if(path==NULL){
                write(1,"$HOME not set\n",15);
                return 1;
            }
        }
    }
    else{
        char **partition = commande->list;
        if(strcmp(partition[1],"-")==0){
            if(previous_directory==NULL){
                write(1,"Il n'y a pas de repertoire de travail précédent\n",50);
                return 1;
            }
            else{
                path = previous_directory;
            }
        }else{
            path = partition[1];
        }
    }
    if(chdir(path)!=0){
        write(1,"Le chemin n'est pas correcte\n",30);
        return 1;
    }else{
        update_directories(path);
        return 0;
    }
}