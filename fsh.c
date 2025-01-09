#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include "commandList.h"
#include "prompt.h"
#include "utils.h"
#include "command.h"
#include "glob.h"
#include "commandList.h"

char *previous_directory = NULL;
char *current_directory = NULL;
int isInRed = 0;
int isOutRed = 0;
int isErrRed = 0;

int sig_int_received = 0;

// Gestionnaire pour SIGINT
void handle_sigint(int sig) {
    if (sig == SIGINT) {
        sig_int_received = 1;
        write(STDERR_FILENO, "\n[SIGINT]\n", 11);
    }
}

// Gestionnaire pour SIGTERM
void handle_sigterm(int sig) {
    // SIGTERM est ignoré
}

// Configuration des gestionnaires de signaux
void setup_signal() {
    struct sigaction sa_int;

    sa_int.sa_handler = handle_sigint;
    //sigemptyset(&sa.sa_mask);
    //sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        perror("Erreur lors de la configuration de SIGINT");
        exit(EXIT_FAILURE);
    }

    struct sigaction sa_term;

    sa_term.sa_handler = handle_sigterm;
    //sigemptyset(&sa.sa_mask);
    //sa.sa_flags = 0;
    if (sigaction(SIGTERM, &sa_term, NULL) == -1) {
        perror("Erreur lors de la configuration de SIGTERM");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv){
    setup_signal();

    int error = 0;
    char *input;
    current_directory = getcwd(NULL,0); // On initialise le répertoire courant
    previous_directory = NULL; // Pour l'instant on n'a pas d'ancien répertoire

    while(1) {
        // Gestion des interruptions par SIGINT
        if (sig_int_received) {
            sig_int_received = 0; 
            error = -1;    
            continue;
        }

        rl_outstream = stderr;
        char *prompt_string = prompt(error);
        input = readline(prompt_string);

        if (input == NULL) break;
        if (strlen(input) == 0) continue;
        if (*input) add_history(input);

        command *commande = newCommand(input);

        free(prompt_string);
        free(input);

        applyCommand(commande, &error);

        freeCommand(commande);
    }

    if (error < 0) exit(255);
    exit(error);
}
