#ifndef REDIRECTION_H
#define REDIRECTION_H

#include "commandList.h"

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

int count_stdin(command *commande);
int count_stdout(command *commande);
int count_stderr(command *commande);

void free_redirection_list(Redirection **redirections, int size);
void free_redirection(Redirection *redirection);


Redirection **get_all_stdin(command *commande, int *size);
Redirection **get_all_stdout(command *commande, int *size);
Redirection **get_all_stderr(command *commande, int *size);

command *remove_all_redirection(command *commande);
int getFile(Redirection *red);

#endif