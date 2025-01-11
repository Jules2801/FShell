#include <stdio.h>
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>
#include <sys/stat.h>

#include "../../include/command.h"
#include "../../include/global.h"

/**
 * Méthode permettant de savoir le type d'un fichier donné.
 * @param commande La commande ayant appelé la méthode et contenant la référence du fichier si correcte.
 * @return 0 si la méthode s'est bien deroulé et 1 sinon.
 */
int cmd_ftype(command *commande) {
    char *errorMessage = "usage: ftype REF\n";
    int size = commande->size;
    char *entry;

    if(size == 0 || size > 2){
        write(2, errorMessage, strlen(errorMessage));
        return 1;
    }

    else if(size == 1){
        char buffer[256];
        if(isInRed && (fgets(buffer, sizeof(buffer), stdin) != NULL)){
            buffer[strcspn(buffer, "\n")] = '\0';
            entry = buffer;
        }
        else{
            write(2, errorMessage, strlen(errorMessage));
            return 1;
        }
    }
    else entry = commande->list[1];

    struct stat statbuf;

    if (lstat(entry, &statbuf) == -1) {
        perror("stat");
        return 1;
    }
    char *s;
    if (S_ISDIR(statbuf.st_mode)) {
        s = "directory\n";
    } else if (S_ISREG(statbuf.st_mode)) {
        s = "regular file\n";
    } else if (S_ISLNK(statbuf.st_mode)) {
        s = "symbolic link\n";
    } else if (S_ISFIFO(statbuf.st_mode)) {
        s = "named pipe\n";
    } else {
        s = "other\n";
    }
    write(1, s, strlen(s));
    return 0;
}