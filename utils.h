#ifndef UTILS_H
#define UTILS_H

char **split_command(char *command);
char ** split_argv_for(int bufSize, char **argv, int argvsize, char *ref, char *varname);
void free_double_buffer(char **buffer, int bufsize);
int partSize(char **argv);
int stringToPositiveInteger(char *arg);
char *extract_extension(char* arg);
char *concat(char *arg1, char *arg2);
int isStructuredCommand(char **partition);
char ***makeStructuredCommand(char **partition, int *size);
char * getRelativeReference(char *dirname, char *filename);
int isSameType(mode_t mode, char f);
char **copyPartition(char **partition);
#endif