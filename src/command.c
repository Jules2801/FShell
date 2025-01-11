#include <signal.h>

#include "../include/utils.h"
#include "../include/global.h"
#include "../include/structures/commandList.h"
#include "../include/structures/redirection.h"

#include "../include/commands/cmd_exit.h"
#include "../include/commands/cmd_cd.h"
#include "../include/commands/cmd_pwd.h"
#include "../include/commands/cmd_ftype.h"
#include "../include/commands/cmd_if.h"
#include "../include/commands/cmd_for.h"

int forCommand(command *command);
int exitCommand(command *command, int *error);
int ftypeCommand(command *command);
int pwdCommand(command *command);
int cdCommand(command *command);
int ifCommand(command *command);
int commande_externe(command *command);

void appliquePipeline(command **commandes,int size,int *error);


/**
 * Méthode permettant de remettre le signal SIGTERM par défaut.
 */
void removeSIGTERM(){
    struct sigaction sa = {0};
    sa.sa_handler = SIG_DFL;
    sigaction(SIGTERM, &sa, NULL);
}

/**
 * Méthode permettant de remettre le signal SIGINT par défaut.
 */
void removeSIGINT(){
    struct sigaction sa = {0};
    sa.sa_handler = SIG_DFL;
    sigaction(SIGINT, &sa, NULL);
}

/**
 * Fonction permettant d'appliquer toutes les commandes internes/externes.
 * @param command Tout ce qui sera utiles pour appliquer la commande, telle que son nom et ses options.
 * @param error Un pointeur d'entier où sera stocké la valeur de retour de la commande fourni.
 */
void applyCommand(command *commande, int *error){
    if(isStructuredCommand(commande,";")){
        int size;
        command **commandes = makeStructuredCommand(commande, &size,";");
        
        for(int i = 0; i < size; i++){
            if(*error != -SIGINT) applyCommand(commandes[i], error);
            freeCommand(commandes[i]);
        }
        free(commandes);
    }

    else if (isStructuredCommand(commande,"|")){
        int size;
        command **commandes = makeStructuredCommand(commande, &size, "|");

        appliquePipeline(commandes, size, error);
    }

    else{
        if(strcmp(commande->list[0], "for") == 0) {
            *error =  forCommand(commande);
            return;
        }
        else if(strcmp(commande->list[0], "if") == 0){
            *error = ifCommand(commande);
            return;
        }
        int countIn = 0;
        int oldIn = dup(STDIN_FILENO);

        int countOut = 0;
        int oldOut = dup(STDOUT_FILENO);

        int countErr = 0;
        int oldErr = dup(STDERR_FILENO);

        Redirection **inList = get_all_stdin(commande, &countIn);
        Redirection **outList = get_all_stdout(commande, &countOut);
        Redirection **errList = get_all_stderr(commande, &countErr);

        if(countIn + countOut + countErr < 0){
            char *s = "Mauvaise utilisation des redirections\n";
            write(2,s,strlen(s));
            *error = 2;
            return;
        }

        command *command_without_red = remove_all_redirection(commande);
        char **partitionWithoutRed = command_without_red->list;

        if(countIn != 0){
            isInRed = 1;
            int fd = getFile(inList[0]);
            if(fd == -1){
                char *s = "Erreur lors de la redirection\n";
                write(2, s, strlen(s));
                *error = 1;
                return;
            }
            dup2(fd, STDIN_FILENO);
        }

        if(countOut != 0){
            isOutRed = 1;
            int fd = getFile(outList[0]);
            if(fd == -1){
                char *s = "Erreur lors de la redirection\n";
                write(2, s, strlen(s));
                *error = 1;
                return;
            }
            dup2(fd, STDOUT_FILENO);
        }

        if(countErr != 0){
            isErrRed = 1;
            int fd = getFile(errList[0]);
            if(fd == -1){
                char *s = "Erreur lors de la redirection\n";
                write(2, s, strlen(s));
                *error = 1;
                return;
            }
            dup2(fd, STDERR_FILENO);
        }

        if(strcmp(partitionWithoutRed[0], "pwd") == 0) *error = pwdCommand(command_without_red);
        else if(strcmp(partitionWithoutRed[0], "exit") == 0) *error = exitCommand(command_without_red, error);
        else if(strcmp(partitionWithoutRed[0], "ftype") == 0) *error = ftypeCommand(command_without_red);
        else if(strcmp(partitionWithoutRed[0], "cd") == 0) *error = cdCommand(command_without_red);
        else *error = commande_externe(command_without_red);

        freeCommand(command_without_red);
        free_redirection_list(inList, countIn);
        free_redirection_list(outList, countOut);
        free_redirection_list(errList, countErr);

        dup2(oldIn, STDIN_FILENO);
        close(oldIn),
        isInRed = 0;

        dup2(oldOut, STDOUT_FILENO);
        close(oldOut);
        isOutRed = 0;

        dup2(oldErr, STDERR_FILENO);
        close(oldErr);
        isErrRed = 0;
    }
}


/**
 * Méthode permettant d'appliquer une/des pipeline(s) si présent.
 * @param commandes Listes des commandes concernés par les pipelines.
 * @param size La taille de la liste commandes.
 * @param error Un pointeur d'entier où sera stockés la valeur de retour.
 */
void appliquePipeline(command **commandes,int size,int *error){
    int i;

    int pipes[size][2];
    pid_t pids[size];

    for (i = 0; i < size ; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }

    for (i = 0; i < size; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork");
            exit(1);
        } else if (pids[i] == 0) {
        
            if (i > 0) {
    
                dup2(pipes[i-1][0], 0);
             
                close(pipes[i-1][0]);
                close(pipes[i-1][1]);
            }

            if (i < size -1) {
              
                dup2(pipes[i][1], 1);
              
                close(pipes[i][0]);
                close(pipes[i][1]);
            }
            for (int j = 0; j < size - 1; j++) {// on nettoie les tubes pas necessaires
                if (j != i - 1 && j != i) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
            }
            applyCommand(commandes[i], error);
            if(*error >= 0)exit(*error);
            exit(255);
        }
    }


    for (i = 0; i < size - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    int status;
    waitpid(pids[size-1], &status, 0);
    if(WIFEXITED(status)) *error = WEXITSTATUS(status);
    else if(WSTOPSIG(status)) *error = -WTERMSIG(status);

    for (i = 0; i < size; i++) {
        freeCommand(commandes[i]);
    }
    free(commandes);
}

/**
 * Fonction servant d'interface pour appliquer la commande for en gérant la sortie de la commande.
 * @param commande Tout ce qui sera utiles pour appliquer la commande externe, telle que son nom et ses options.
 * @return Un entier supérieure à 1 si une erreur s'est produite pendant le processus, 0 si tout s'est bien passé et une valeur négative si un signal s'est levé.
 */
int forCommand(command *commande){
    pid_t pid = 0;
    if((pid = fork()) == 0){
        removeSIGINT();
        removeSIGTERM();
        linkedList *lcommandes = newLinkedList(NULL);
        int parrallel;
        int ret = cmd_for(commande,lcommandes, &parrallel);
        if(ret != 0){
            freeLinkedList(lcommandes);
            exit(ret);
        }
        int res = 0;
        if(parrallel == -1){
            while(lcommandes->head != NULL){
                command *cmd = getCommand(lcommandes);
                int temp = 0;
                if(res != -SIGINT) applyCommand(cmd, &temp);
                if((temp > res && res != -SIGINT) || temp == -SIGINT) res = temp;
                itere(lcommandes);
            }
        }
        else{
            int active_process = 0;
            // Si l'option -p est activée  
            while(lcommandes->head != NULL){
                if (active_process < parrallel) {
                pid_t pid = fork();
                if (pid == 0) {
                    int temp = 0;
                    if(res != -SIGINT) applyCommand(lcommandes->head->data, &temp);
                    if((temp > res && res != -SIGINT) || temp == -SIGINT) res = temp;
                    freeLinkedList(lcommandes);
                    exit(res);
                } else if (pid > 0) {
                    active_process++;
                    itere(lcommandes);
                }   else {
                    freeLinkedList(lcommandes);
                    exit(1);
                }

            } else {
                int status;
                wait(&status);
                if(WIFEXITED(status)) if(WEXITSTATUS(status) > res) res = WEXITSTATUS(status);
                active_process--;
                }
            }

            while (active_process > 0) {
                int status;
                wait(&status);
                if(WIFEXITED(status)) if(WEXITSTATUS(status) > res) res = WEXITSTATUS(status);
                active_process--;
            }

        }
        freeLinkedList(lcommandes);
        if(res < 0) raise(-res);
        exit(res);
    }
    else{
        int status;
        waitpid(pid, &status, 0);
        if(WIFEXITED(status)) return WEXITSTATUS(status);
        if(WIFSIGNALED(status)) return -WTERMSIG(status);
        return 1;
    }
}

/**
 * Fonction servant d'interface pour appliquer la commande for en gérant la sortie de la commande.
 * @param commande Tout ce qui sera utiles pour appliquer la commande externe, telle que son nom et ses options.
 * @return Un entier supérieure à 1 si une erreur s'est produite pendant le processus, 0 si tout s'est bien passé et une valeur négative si un signal s'est levé.
 */
int ifCommand(command *commande){
    pid_t pid = 0;
    if((pid = fork()) == 0){
        removeSIGTERM();
        removeSIGINT();
        int res = cmd_if(commande);
        if(res < 0) raise(-res);
        exit(res);
    }
    else{
        int status;
        waitpid(pid, &status, 0);
        if(WIFEXITED(status)) return WEXITSTATUS(status);
        if(WIFSIGNALED(status)) return -WTERMSIG(status);
        return 1;
    }
}

/**
 * Fonction servant d'interface pour appliquer la commande for en gérant la sortie de la commande.
 * @param commande Tout ce qui sera utiles pour appliquer la commande externe, telle que son nom et ses options.
 * @return Un entier supérieure à 1 si une erreur s'est produite pendant le processus, 0 si tout s'est bien passé et une valeur négative si un signal s'est levé.
 */
int pwdCommand(command *command){
    pid_t pid = 0;
    if((pid = fork()) == 0){
        removeSIGTERM();
        int res = cmd_pwd(command);
        if(res < 0) raise(-res);
        exit(res);
    }
    else{
        int status;
        waitpid(pid, &status, 0);
        if(WIFEXITED(status)) return WEXITSTATUS(status);
        if(WIFSIGNALED(status)) return -WTERMSIG(status);
        return 1;
    }
}

/**
 * Fonction servant d'interface pour appliquer la commande for en gérant la sortie de la commande.
 * @param commande Tout ce qui sera utiles pour appliquer la commande externe, telle que son nom et ses options.
 * @param error Un pointeur d'entier qui pourrait servir à l'exécution de la commande exit.
 * @return Un entier supérieure à 1 si une erreur s'est produite pendant le processus, 0 si tout s'est bien passé et une valeur négative si un signal s'est levé.
 */
int exitCommand(command *commande, int *error){
    return cmd_exit(commande, error);
}

/**
 * Fonction servant d'interface pour appliquer la commande for en gérant la sortie de la commande.
 * @param commande Tout ce qui sera utiles pour appliquer la commande externe, telle que son nom et ses options.
 * @return Un entier supérieure à 1 si une erreur s'est produite pendant le processus, 0 si tout s'est bien passé et une valeur négative si un signal s'est levé.
 */
int ftypeCommand(command *commande){
    pid_t pid = 0;
    if((pid = fork()) == 0){
        removeSIGTERM();
        int res = cmd_ftype(commande);
        if(res < 0) raise(-res);
        exit(res);
    }
    else{
        int status;
        waitpid(pid, &status, 0);
        if(WIFEXITED(status)) return WEXITSTATUS(status);
        if(WIFSIGNALED(status)) return -WTERMSIG(status);
        return 1;
    }
}

/**
 * Fonction servant d'interface pour appliquer la commande for en gérant la sortie de la commande.
 * @param commande Tout ce qui sera utiles pour appliquer la commande externe, telle que son nom et ses options.
 * @return Un entier supérieure à 1 si une erreur s'est produite pendant le processus, 0 si tout s'est bien passé et une valeur négative si un signal s'est levé.
 */
int cdCommand(command *commande){
    return cmd_cd(commande);   
}

/**
 * Fonction permettant d'exécuter une commande externe (si c'est le cas) donné en argument.
 * @param command Tout ce qui sera utiles pour appliquer la commande externe, telle que son nom et ses options.
 * @return Un entier supérieure à 1 si une erreur s'est produite pendant le recouvrement et 0 si tout s'est bien passé.
 */
int commande_externe(command *commande){
    pid_t pid = fork();
    if(pid == -1){
        perror("Erreur lors de la création du processus");
        return -1;
    }

    if(pid == 0){
        removeSIGTERM();
        char **partition = commande->list;

        if(strlen(partition[0]) > 2 && partition[0][0] == '.' && partition[0][1] == '/') execv(partition[0], partition);
        execvp(partition[0], partition);
        char *s = concat("fsh: command not found: ", partition[0]);
        write(2, s, strlen(s));
        free(s);
        exit(1);
    }

    else{
        int status;
        waitpid(pid, &status, 0);
        if(WIFEXITED(status)) return WEXITSTATUS(status);
        if(WIFSIGNALED(status)) return -WTERMSIG(status);
        return 1;
    }
}