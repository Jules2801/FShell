#ifndef REDIRECTION_H
#define REDIRECTION_H

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

int count_stdin(char **command);
int count_stdout(char **command);
int count_stderr(char **command);

void free_redirection_list(Redirection **redirections, int size);
void free_redirection(Redirection *redirection);


Redirection **get_all_stdin(char **command, int *size);
Redirection **get_all_stdout(char **command, int *size);
Redirection **get_all_stderr(char **command, int *size);

char **remove_all_redirection(char **command);
int getFile(Redirection *red);

#endif