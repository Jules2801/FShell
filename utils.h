#ifndef UTILS_H
#define UTILS_H

char **split_command(char *command);
char ** split_argv_for(int bufSize, char **argv, char *filename, char *dirname);
int applyCommand(char **command);

#endif