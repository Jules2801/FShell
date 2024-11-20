#ifndef UTILS_H
#define UTILS_H

char **split_command(char *command);
char ** split_argv_for(int bufSize, char **argv, int argvsize, char *filename, char *dirname, char *varname);
int partSize(char **argv);
#endif