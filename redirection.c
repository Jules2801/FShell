#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <dirent.h>
#include "utils.h"
#include <sys/stat.h>
#include "global.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "commandList.h"
#include "redirection.h"



int is_stdout(char *entry){
    return strcmp(entry, ">") == 0
    || strcmp(entry, ">|") == 0
    || strcmp(entry, ">>") == 0;
}

int is_stderr(char *entry){
    return strcmp(entry, "2>") == 0
    || strcmp(entry, "2>|") == 0
    || strcmp(entry, "2>>") == 0;
}

int is_stdin(char *entry){
    return strcmp(entry, "<") == 0;
}

int count_stdin(command *commande){
    int size = commande->size;
    int count = 0;
    char **partition = commande->list;

    int hasSemi = 0;
    int hasBrace = 0;
    int hasQuote = 0;

    for(int i = 1; i < size; i++){
        if(strcmp(partition[i], "{") == 0) hasBrace = 1;
        else if(strcmp(partition[i], "}") == 0) hasBrace = 0;
        else if(strstr(partition[i], "\"") != NULL){
            if(hasQuote == 0) hasQuote = 1;
            else hasQuote = 0;
        }
        else if (is_stdin(partition[i]) && hasBrace == 0 && hasQuote == 0) {
            if(i+1 < size && !isIllegalName(partition[i+1])) count++;
            else return -1;
        }
    }

    return count;
}

int count_stdout(command *commande){
    int size = commande->size;
    int count = 0;
    char **partition = commande->list;

    int hasSemi = 0;
    int hasBrace = 0;
    int hasQuote = 0;

    for(int i = 1; i < size; i++){
        if(strcmp(partition[i], "{") == 0) hasBrace = 1;
        else if(strcmp(partition[i], "}") == 0) hasBrace = 0;
        else if(strstr(partition[i], "\"") != NULL){
            if(hasQuote == 0) hasQuote = 1;
            else hasQuote = 0;
        }
        else if (is_stdout(partition[i]) && hasBrace == 0 && hasQuote == 0){
            if(i+1 < size && !isIllegalName(partition[i+1])) count++;
            else return -1;
        }
    }

    return count;
}

int count_stderr(command *commande){
    int size = commande->size;
    int count = 0;
    char **partition = commande->list;

    int hasSemi = 0;
    int hasBrace = 0;
    int hasQuote = 0;

    for(int i = 1; i < size; i++){
        if(strcmp(partition[i], "{") == 0) hasBrace = 1;
        else if(strcmp(partition[i], "}") == 0) hasBrace = 0;
        else if(strstr(partition[i], "\"") != NULL){
            if(hasQuote == 0) hasQuote = 1;
            else hasQuote = 0;
        }
        else if (is_stderr(partition[i]) && hasBrace == 0 && hasQuote == 0){
            if(i+1 < size && !isIllegalName(partition[i+1])) count++;
            else return -1;
        }
    }

    return count;
}

void free_redirection(Redirection *redirection){
    free(redirection->filename);
    free(redirection);
}

Redirection *make_redirection(char *redtype, char *filename){
    Redtype type;
    if(is_stdin((redtype))) type = RSTDIN;
    else if(is_stdout(redtype)){
        if(strcmp(redtype, ">") == 0) type = RSTDOUT;
        else if(strcmp(redtype, ">|") == 0) type = RSTDOUT_WITH_CRUSHING;
        else type = RSTDOUT_WITH_APPEND;
    }
    else if(is_stderr(redtype)){
        if(strcmp(redtype, "2>") == 0) type = RSTDERR;
        else if(strcmp(redtype, "2>|") == 0) type = RSTDERR_WITH_CRUSHING;
        else type = RSTDERR_WITH_APPEND;
    }
    else{
        char *s = "Erreur lors de la lecture du type de redirection\n";
        write(2, s, strlen(s));
        exit(1);
    }

    Redirection *red;
    if((red = malloc(sizeof(Redirection))) == NULL){
        char *s = "Erreur lors de l'allocation mémoire de la redirection\n";
        write(2, s, strlen(s));
        exit(1);
    }

    if((red->filename = malloc(strlen(filename) + 1)) == NULL){
        char *s = "Erreur lors de l'allocation mémoire de la redirection\n";
        write(2, s, strlen(s));
        free(red);
        exit(1);
    }

    strcpy(red->filename, filename);
    red->type = type;

    return red;
}

int getFile(Redirection *red){
    int fd = -1;
    switch(red->type){
        case RSTDIN: 
            fd = open(red->filename, O_RDONLY);
            break;
        case RSTDOUT:
            fd = open(red->filename, O_WRONLY | O_EXCL | O_CREAT, 0644);
            break;
        case RSTDOUT_WITH_CRUSHING:
            fd = open(red->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            break;
        case RSTDOUT_WITH_APPEND:
            fd = open(red->filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
            break;
        case RSTDERR:
            fd = open(red->filename, O_WRONLY | O_EXCL | O_CREAT, 0644);
            break;
        case RSTDERR_WITH_CRUSHING:
            fd = open(red->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            break;
        case RSTDERR_WITH_APPEND:
            fd = open(red->filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
            break;
    }
    return fd;
}

void free_redirection_list(Redirection **redirections, int size){
    for(int i = 0; i < size; i++){
        free_redirection(redirections[i]);
    }
    free(redirections);
}

Redirection **get_all_stdin(command *commande, int *listSize){
    int count = count_stdin(commande);
    if(count <= 0){
        *listSize = count;
        return NULL;
    }

    Redirection **res;
    if((res = malloc(count * sizeof(Redirection *))) == NULL){
        char *s = "Erreur lors de l'allocation mémoire\n";
        write(2, s, strlen(s));
        return NULL;
    }

    int size = commande->size;

    int hasSemi = 0;
    int hasBrace = 0;
    int hasQuote = 0;

    char **partition = commande->list;

    int pos = 0;

    for(int i = 1; i < size; i++){
        if(strcmp(partition[i], "{") == 0) hasBrace = 1;
        else if(strcmp(partition[i], "}") == 0) hasBrace = 0;
        else if(strstr(partition[i], "\"") != NULL){
            if(hasQuote == 0) hasQuote = 1;
            else hasQuote = 0;
        }
        else if (is_stdin(partition[i]) && hasBrace == 0 && hasQuote == 0){
            if(i+1 >= size){
                char *s = "Mauvais usage de la redirection\n";
                write(2, s, strlen(s));
                free_redirection_list(res, pos);
                return NULL;
            }
            res[pos] = make_redirection(partition[i], partition[i+1]);
            pos++;
        }
    }

    *listSize = pos;
    return res;
}

Redirection **get_all_stdout(command *commande, int *listSize){
    int count = count_stdout(commande);
    if(count <= 0){
        *listSize = count;
        return NULL;
    }

    Redirection **res;
    if((res = malloc(count * sizeof(Redirection *))) == NULL){
        char *s = "Erreur lors de l'allocation mémoire\n";
        write(2, s, strlen(s));
        return NULL;
    }

    int size = commande->size;

    char **partition = commande->list;

    int hasSemi = 0;
    int hasBrace = 0;
    int hasQuote = 0;

    int pos = 0;

    for(int i = 1; i < size; i++){
        if(strcmp(partition[i], "{") == 0) hasBrace = 1;
        else if(strcmp(partition[i], "}") == 0) hasBrace = 0;
        else if(strstr(partition[i], "\"") != NULL){
            if(hasQuote == 0) hasQuote = 1;
            else hasQuote = 0;
        }
        else if (is_stdout(partition[i]) && hasBrace == 0 && hasQuote == 0){
            if(i+1 >= size){
                char *s = "Mauvais usage de la redirection\n";
                write(2, s, strlen(s));
                free_redirection_list(res, pos);
                return NULL;
            }
            res[pos] = make_redirection(partition[i], partition[i+1]);
            pos++;
        }
    }

    *listSize = pos;
    return res;
}

Redirection **get_all_stderr(command *commande, int *listSize){
    int count = count_stderr(commande);
    if(count <= 0){
        *listSize = count;
        return NULL;
    }

    Redirection **res;
    if((res = malloc(count * sizeof(Redirection *))) == NULL){
        char *s = "Erreur lors de l'allocation mémoire\n";
        write(2, s, strlen(s));
        return NULL;
    }

    int size = commande->size;

    char **partition = commande->list;

    int hasSemi = 0;
    int hasBrace = 0;
    int hasQuote = 0;

    int pos = 0;

    for(int i = 1; i < size; i++){
        if(strcmp(partition[i], "{") == 0) hasBrace = 1;
        else if(strcmp(partition[i], "}") == 0) hasBrace = 0;
        else if(strstr(partition[i], "\"") != NULL){
            if(hasQuote == 0) hasQuote = 1;
            else hasQuote = 0;
        }
        else if (is_stderr(partition[i]) && hasBrace == 0 && hasQuote == 0){
            if(i+1 >= size){
                char *s = "Mauvais usage de la redirection\n";
                write(2, s, strlen(s));
                free_redirection_list(res, pos);
                return NULL;
            }
            res[pos] = make_redirection(partition[i], partition[i+1]);
            pos++;
        }
    }

    *listSize = pos;
    return res;
}

command *remove_all_redirection(command *commande){
    if (commande == NULL || commande ->list == NULL) return NULL;
    int count_red = count_stdin(commande) + count_stdout(commande) + count_stderr(commande);
    if(count_red == 0) return copyPartition(commande);

    int size = commande->size;
    int newSize = size - count_red * 2;
    int pos = 0;

    char **partition = commande->list;

    char **newPart = malloc((newSize+1) * sizeof(char *));
    if(newPart == NULL){
        char *s = "Erreur lors de l'allocation mémoire\n";
        write(2, s, strlen(s));
        return NULL;
    }

    int i = 0;
    while(i < size){
        if(is_stdin(partition[i]) || is_stdout(partition[i]) || is_stderr(partition[i])) i+= 2;
        else{
            if((newPart[pos] = malloc(strlen(partition[i]) + 1)) == NULL){
                char *s = "Erreur lors de l'allocation mémoire\n";
                write(2, s, strlen(s));
                free_double_buffer(newPart, pos);
                return NULL;
            }
            strcpy(newPart[pos], partition[i]);
            pos++;
            i++;
        }
    }

    newPart[newSize] = (char *) NULL;

    command *res = newCommandList(newPart);

    return res;
}