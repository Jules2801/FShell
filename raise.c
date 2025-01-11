#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <signal.h>
#include <assert.h>
#include <signal.h>


int main(int argc, char **argv){
    assert(argc == 2);

    if(strcmp(argv[1], "TERM") == 0) raise(SIGTERM);
    if(strcmp(argv[1], "INT") == 0) raise(SIGINT);

    exit(0);
}