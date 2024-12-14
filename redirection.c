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

typedef enum {
    RSTDIN,
    RSTDOUT,
    RSTDOUT_WITH_CRUSHING,
    RSTDOUT_WITH_APPEND,
    RSTDERR,
    RSTDERR_WITH_CRUSHING,
    RSTDERR_WITH_APPEND
} Redtype;

typedef struct {
    Redtype type;
    char *filename;
} Redirection;

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

int count_stdin(char **command){
    int size = partSize(command);
    int count = 0;

    int hasSemi = 0;
    int hasBrace = 0;
    int hasQuote = 0;

    for(int i = 1; i < size; i++){
        if(strcmp(command[i], "{") == 0) hasBrace = 1;
        else if(strcmp(command[i], "}") == 0) hasBrace = 0;
        else if(strstr(command[i], "\"") != NULL){
            if(hasQuote == 0) hasQuote = 1;
            else hasQuote = 0;
        }
        else if (is_stdin(command[i]) && hasBrace == 0 && hasQuote == 0) count++;
    }

    return count;
}

int count_stdout(char **command){
    int size = partSize(command);
    int count = 0;

    int hasSemi = 0;
    int hasBrace = 0;
    int hasQuote = 0;

    for(int i = 1; i < size; i++){
        if(strcmp(command[i], "{") == 0) hasBrace = 1;
        else if(strcmp(command[i], "}") == 0) hasBrace = 0;
        else if(strstr(command[i], "\"") != NULL){
            if(hasQuote == 0) hasQuote = 1;
            else hasQuote = 0;
        }
        else if (is_stdout(command[i]) && hasBrace == 0 && hasQuote == 0) count++;
    }

    return count;
}

int count_stderr(char **command){
    int size = partSize(command);
    int count = 0;

    int hasSemi = 0;
    int hasBrace = 0;
    int hasQuote = 0;

    for(int i = 1; i < size; i++){
        if(strcmp(command[i], "{") == 0) hasBrace = 1;
        else if(strcmp(command[i], "}") == 0) hasBrace = 0;
        else if(strstr(command[i], "\"") != NULL){
            if(hasQuote == 0) hasQuote = 1;
            else hasQuote = 0;
        }
        else if (is_stderr(command[i]) && hasBrace == 0 && hasQuote == 0) count++;
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

Redirection **get_all_stdin(char **command, int *listSize){
    int count = count_stdin(command);
    if(count_stdin(command) == 0) return NULL;

    Redirection **res;
    if((res = malloc(count * sizeof(Redirection *))) == NULL){
        char *s = "Erreur lors de l'allocation mémoire\n";
        write(2, s, strlen(s));
        exit(0);
    }

    int size = partSize(command);

    int hasSemi = 0;
    int hasBrace = 0;
    int hasQuote = 0;

    int pos = 0;

    for(int i = 1; i < size; i++){
        if(strcmp(command[i], "{") == 0) hasBrace = 1;
        else if(strcmp(command[i], "}") == 0) hasBrace = 0;
        else if(strstr(command[i], "\"") != NULL){
            if(hasQuote == 0) hasQuote = 1;
            else hasQuote = 0;
        }
        else if (is_stdin(command[i]) && hasBrace == 0 && hasQuote == 0){
            if(i+1 >= size){
                char *s = "Mauvais usage de la redirection\n";
                write(2, s, strlen(s));
                free_redirection_list(res, pos);
                exit(1);
            }
            res[pos] = make_redirection(command[i], command[i+1]);
            pos++;
        }
    }

    *listSize = pos;
    return res;
}

Redirection **get_all_stdout(char **command, int *listSize){
    int count = count_stdin(command);
    if(count_stdout(command) == 0) return NULL;

    Redirection **res;
    if((res = malloc(count * sizeof(Redirection *))) == NULL){
        char *s = "Erreur lors de l'allocation mémoire\n";
        write(2, s, strlen(s));
        exit(0);
    }

    int size = partSize(command);

    int hasSemi = 0;
    int hasBrace = 0;
    int hasQuote = 0;

    int pos = 0;

    for(int i = 1; i < size; i++){
        if(strcmp(command[i], "{") == 0) hasBrace = 1;
        else if(strcmp(command[i], "}") == 0) hasBrace = 0;
        else if(strstr(command[i], "\"") != NULL){
            if(hasQuote == 0) hasQuote = 1;
            else hasQuote = 0;
        }
        else if (is_stdout(command[i]) && hasBrace == 0 && hasQuote == 0){
            if(i+1 >= size){
                char *s = "Mauvais usage de la redirection\n";
                write(2, s, strlen(s));
                free_redirection_list(res, pos);
                exit(1);
            }
            res[pos] = make_redirection(command[i], command[i+1]);
            pos++;
        }
    }

    *listSize = pos;
    return res;
}

Redirection **get_all_stderr(char **command, int *listSize){
    int count = count_stdin(command);
    if(count_stderr(command) == 0) return NULL;

    Redirection **res;
    if((res = malloc(count * sizeof(Redirection *))) == NULL){
        char *s = "Erreur lors de l'allocation mémoire\n";
        write(2, s, strlen(s));
        exit(0);
    }

    int size = partSize(command);

    int hasSemi = 0;
    int hasBrace = 0;
    int hasQuote = 0;

    int pos = 0;

    for(int i = 1; i < size; i++){
        if(strcmp(command[i], "{") == 0) hasBrace = 1;
        else if(strcmp(command[i], "}") == 0) hasBrace = 0;
        else if(strstr(command[i], "\"") != NULL){
            if(hasQuote == 0) hasQuote = 1;
            else hasQuote = 0;
        }
        else if (is_stderr(command[i]) && hasBrace == 0 && hasQuote == 0){
            if(i+1 >= size){
                char *s = "Mauvais usage de la redirection\n";
                write(2, s, strlen(s));
                free_redirection_list(res, pos);
                exit(1);
            }
            res[pos] = make_redirection(command[i], command[i+1]);
            pos++;
        }
    }

    *listSize = pos;
    return res;
}

char **remove_all_redirection(char **command){
    if (command == NULL || command[0] == NULL) return NULL;
    int count_red = count_stdin(command) + count_stdout(command) + count_stderr(command);
    if(count_red == 0) return copyPartition(command);

    int size = partSize(command);
    int newSize = size - count_red * 2;
    int pos = 0;

    char **res = malloc((newSize+1) * sizeof(char *));
    if(res == NULL){
        char *s = "Erreur lors de l'allocation mémoire\n";
        write(2, s, strlen(s));
        exit(1);
    }

    int i = 0;
    while(i < size){
        if(is_stdin(command[i]) || is_stdout(command[i]) || is_stderr(command[i])) i+= 2;
        else{
            if((res[pos] = malloc(strlen(command[i]) + 1)) == NULL){
                char *s = "Erreur lors de l'allocation mémoire\n";
                write(2, s, strlen(s));
                free_double_buffer(res, pos);
                exit(1);
            }
            strcpy(res[pos], command[i]);
            pos++;
            i++;
        }
    }

    res[newSize] = (char *) NULL;

    return res;
}