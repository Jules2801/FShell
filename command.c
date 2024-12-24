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

int commande_externe(command *command);

int forCommand(command *command);
int cmd_for(command *command,linkedList *commandList, int *parralel);

int exitCommand(command *command, int *error);
int cmd_exit(command *command, int *error);

int ftypeCommand(command *command);
int cmd_ftype(command *command);

int pwdCommand(command *command);
int cmd_pwd(command *command);

int cdCommand(command *command);
int cmd_cd(command *command);

int ifCommand(command *command);
int cmd_if(command *commande);

/**
 * Fonction permettant de mettre a jour le repertoire courant et l'ancien
 * @param new_directory le chemin du nouveau repertoire courant
 */
void update_directories(const char *new_directory) {
    if(new_directory == NULL) {
        char *s = "Erreur: le nouveau répertoire est NULL.\n";
        write(2, s, strlen(s));
        return;
    }

    char *new_directory_copy = strdup(new_directory);
    if (new_directory_copy == NULL) {
        char *s = "Erreur: Échec de l'allocation mémoire pour le nouveau répertoire.\n";
        write(2, s, strlen(s));
        return;
    }

    if (previous_directory != NULL) {
        free(previous_directory);
    }

    previous_directory = current_directory;
    current_directory = new_directory_copy;
}

/**
 * Fonction permettant d'appliquer toutes les fonctions internes/externes.
 * @param command Tout ce qui sera utiles pour appliquer la commande externe, telle que son nom et ses options.
 */
void applyCommand(command *commande, int *error){
    if(isStructuredCommand(commande)){
        int size;
        command **commandes = makeStructuredCommand(commande, &size);
        
        for(int i = 0; i < size; i++){
            applyCommand(commandes[i], error);
            freeCommand(commandes[i]);
        }
        free(commandes);
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
 * Fonction permettant d'appliquer la commande for.
 * @param command Tout ce qui sera utiles pour appliquer la commande externe, telle que son nom et ses options.
 * @param size Taille de la partition de chaîne de caractère.
 * @return Un entier supérieure à 1 si une erreur s'est produite pendant le processus et 0 si tout s'est bien passé.
 */
int forCommand(command *commande){
    pid_t pid = 0;
    if((pid = fork()) == 0){
        linkedList *lcommandes = NULL;
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
                int temp;
                applyCommand(cmd, &temp);
                if(temp > res) res = temp;
                itere(lcommandes);
                
            }
            exit(res);
        }
        else{
            int active_process = 0;
            // Si l'option -p est activée  
    while(lcommandes->head != NULL){
        if (active_process < parrallel) {
        pid_t pid = fork();
        if (pid == 0) {
            int temp;
            applyCommand(lcommandes->head->data, &temp);
            if(temp > res) res = temp;
            
            exit(res);
        } else if (pid > 0) {
            active_process++;
            itere(lcommandes);

        }
    } else {
        wait(NULL);
        active_process--;
        }
    }

    while (active_process > 0) {
        wait(NULL);
        active_process--;
    }

        }
        freeLinkedList(lcommandes);
        exit(0);
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
 * Fonction permettant d'appliquer la commande if ( if TEST { CMD_1 } ou f TEST { CMD_1 } else { CMD_2 }).
 * @return Un entier supérieure à 1 si une erreur s'est produite pendant le processus et 0 si tout s'est bien passé.
 */
int ifCommand(command *commande){
    pid_t pid = 0;
    if((pid = fork()) == 0){
        exit(cmd_if(commande));
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
 * Fonction permettant d'appliquer la commande pwd.
 * @return Un entier supérieure à 1 si une erreur s'est produite pendant le processus et 0 si tout s'est bien passé.
 */
int pwdCommand(command *command){
    pid_t pid = 0;
    if((pid = fork()) == 0){
        exit(cmd_pwd(command));
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
 * Fonction permettant d'appliquer la commande exit.
 * @return Un entier supérieure à 1 si une erreur s'est produite pendant le processus et 0 si tout s'est bien passé.
 */
int exitCommand(command *commande, int *error){
    return cmd_exit(commande, error);
}

/**
 * Fonction permettant d'appliquer la commande exit.
 * @return Un entier supérieure à 1 si une erreur s'est produite pendant le processus et 0 si tout s'est bien passé.
 */
int ftypeCommand(command *commande){
    pid_t pid = 0;
    if((pid = fork()) == 0){
        exit(cmd_ftype(commande));
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
 * Fonction permettant d'appliquer la commande cd.
 * @return Un entier supérieure à 1 si une erreur s'est produite pendant le processus et 0 si tout s'est bien passé.
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


int cmd_exit(command *commande, int *error) {
    char *errorMessage = "usage: exit [VAL]\n";
    int size = commande->size;

    if(size == 0){
        write(2, errorMessage, strlen(errorMessage));
        return 1;
    }
    else if(size == 1){
        if(*error < 0) exit(255);
        else exit(*error);
    }
    else if(size == 2){
        int err = stringToPositiveInteger(commande->list[1]);
        if(err == -1){
            write(2, errorMessage, strlen(errorMessage));
            return 1;
        }
        else exit(err);
    }
    else{
        char *s = "exit: trop d'arguments.\n";
        write(2, s, strlen(s));
        return 1;
    }
}

int cmd_ftype(command *commande) {
    char *errorMessage = "usage: ftype REF\n";
    int size = commande->size;
    char *entry;

    if(size == 0 || size > 2){
        write(2, errorMessage, strlen(errorMessage));
        return 1;
    }

    else if(size == 1){
        char buffer[256];
        if(isInRed && (fgets(buffer, sizeof(buffer), stdin) != NULL)){
            buffer[strcspn(buffer, "\n")] = '\0';
            entry = buffer;
        }
        else{
            write(2, errorMessage, strlen(errorMessage));
            return 1;
        }
    }
    else entry = commande->list[1];

    struct stat statbuf;

    if (lstat(entry, &statbuf) == -1) {
        perror("stat");
        return 1;
    }
    char *s;
    if (S_ISDIR(statbuf.st_mode)) {
        s = "directory\n";
    } else if (S_ISREG(statbuf.st_mode)) {
        s = "regular file\n";
    } else if (S_ISLNK(statbuf.st_mode)) {
        s = "symbolic link\n";
    } else if (S_ISFIFO(statbuf.st_mode)) {
        s = "named pipe\n";
    } else {
        s = "other\n";
    }
    write(1, s, strlen(s));
    return 0;
}

int cmd_pwd(command *commande) {

    char cwd[1024];

    if(commande->size != 1){
        char *s = "pwd: trop d'arguments\n";
        write(2, s, strlen(s));
        return 1;
    }

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        strcat(cwd, "\n");
        write(1, cwd, strlen(cwd));
        return 0;
    } else {
        perror("getcwd");
        return 1;
    }
}

int cmd_cd(command *commande){
    char *path;
    int partSize = commande->size;
    if(partSize>2){
        char *s = "cd: trop d'arguments\n";
        write(2, s, strlen(s));
        return 1;
    }
    if(partSize==1){
        char buffer[256];
        if(isInRed && (fgets(buffer, sizeof(buffer), stdin) != NULL)){
            buffer[strcspn(buffer, "\n")] = '\0';
            if (strlen(buffer) == 0) {
                write(1, "Chemin non valide ou vide\n", 26);
                return 1;
            }
            path = buffer;
        }
        else{
            path = getenv("HOME");
            if(path==NULL){
                write(1,"$HOME not set\n",15);
                return 1;
            }
        }
    }
    else{
        char **partition = commande->list;
        if(strcmp(partition[1],"-")==0){
            if(previous_directory==NULL){
                write(1,"Il n'y a pas de repertoire de travail précédent\n",50);
                return 1;
            }
            else{
                path = previous_directory;
            }
        }else{
            path = partition[1];
        }
    }
    if(chdir(path)!=0){
        write(1,"Le chemin n'est pas correcte\n",30);
        return 1;
    }else{
        update_directories(path);
        return 0;
    }
}

int cmd_if(command *commande){
    int argc = commande->size;
    char  **argv = commande->list;

    int result = 0;
    int test_result;
    char *errorMessage = "usage: if [ TEST ] { CMD } or if [ TEST ] { CMD_1 } else { CMD_2 } or if test condition { CMD }...\n";

    if(argc < 5){
        write(2, errorMessage, strlen(errorMessage));
        return 2;
    }

    int first_opening_bracket_pos;
    for (int i = 2; i < argc; i++) { // On cherche la position de la première `{`
        if (strcmp("{", argv[i]) == 0) {
            first_opening_bracket_pos = i;
            break;
        }
    }

    if(strcmp("}", argv[argc-1]) != 0 || strcmp("{", argv[first_opening_bracket_pos]) != 0 || isBraceGood(argv) == 0 || isBraceGood2(argv) == 0){
        write(2, errorMessage, strlen(errorMessage));
        return 2;
    }

    int else_pos = argc;
    hasElse(argv, &else_pos); // Verifie si la commande contient un else
    if(else_pos != argc){
        if(strcmp("}", argv[else_pos - 1]) != 0 || strcmp("{", argv[else_pos + 1]) != 0 ){
            write(2, errorMessage, strlen(errorMessage));
            return 2;
        }
    }
    
    command *cmd_test = makeSubPartition(commande,1,first_opening_bracket_pos -1);
    applyCommand(cmd_test, &test_result);
    command *cmd_1 = NULL;
    command *cmd_2 = NULL;

    if(test_result == 0){
        cmd_1 = makeSubPartition(commande,first_opening_bracket_pos + 1, else_pos - 2);
        applyCommand(cmd_1, &result);
        goto cleanup_return;
    }else{
        if(else_pos != argc){
            cmd_2 = makeSubPartition(commande, else_pos + 2, argc - 2);
            applyCommand(cmd_2, &result);
            goto cleanup_return;
        }   
    }

    goto cleanup_return;

    cleanup_return :
        if(cmd_test != NULL) freeCommand(cmd_test); 
        if(cmd_1 != NULL) freeCommand(cmd_1);
        if(cmd_2 != NULL) freeCommand(cmd_2);
    return result;
}

int cmd_for(command *commande,linkedList *commandList, int *parralel){
    int result = 0;
    char *errorMessage = "usage: for [variable-name] in REP [-A] [-r] [-e EXT] [-t TYPE] [-p MAX] { CMD }\n";
    char buf[1024];

    DIR *dir = NULL;
    char *varname = NULL;

    int a_option = 0;
    int r_option = 0;
    char *extension = NULL;
    char type = '\0';
    int max_parralel = -1;

    struct dirent *entry = NULL;

    int argc = commande->size;
    char **argv = commande->list;

    if(argc < 7 || isIllegalName(argv[3])){
        write(2, errorMessage, strlen(errorMessage));
        return 2;
    }

    if(getcwd(buf, 1024) == NULL){
        perror("Erreur lors de l'utilisation de getcwd");
        return 1;
    }
    char *ref = getRelativeReference(".", argv[3]);
    if((dir = opendir(ref)) == NULL){
        perror("Erreur lors de l'ouverture du fichier");
        result = 1;
        goto cleanup_return;
    }
    free(ref);

    if((varname = malloc((strlen(argv[1]) + 2) * sizeof(char))) == NULL){
        perror("Erreur lors de l'allocation mémoire");
        result = 1;
        goto cleanup_return;
    }
    varname[0] = '\0';
    if(strcat(varname, "$") == NULL){
        perror("Erreur lors de la concaténation d'une chaîne de caractère");
        result = 1;
        goto cleanup_return;
    }
    if(strcat(varname, argv[1]) == NULL){
        perror("Erreur lors de la concaténation d'une chaîne de caractère");
        result = 1;
        goto cleanup_return;
    }

    int opt;
    #ifdef __APPLE__
        optind = 1;
    #else
        optind = 0;
    #endif
    
    while((opt = getopt(argc, argv+3, "+:Are:t:p:")) != -1){
        switch(opt){
            case 'A':
                a_option = 1;
                fflush(stdout);
                break;

            case 'r':
                r_option = 1;
                break;

            case 'e' :
                if((extension = malloc(strlen(optarg) * sizeof(char) + 1)) == NULL){
                    char *s = "Erreur lors de l'allocation mémoire\n";
                    write(STDERR_FILENO, s, strlen(s));
                    result = 1;
                    goto cleanup_return;
                }

                strcpy(extension, optarg);
                *(extension + strlen(optarg)) = '\0';

                break;

            case 't' :
                if(strlen(optarg) != 1 || (*optarg != 'f' && *optarg != 'd' && *optarg != 'l' && *optarg != 'p')){
                    char *s = "Vous devez fournir un caractère valide (f pour les fichiers ordinaires, d pour les répertires, l pour les liens symboliques, p pour les tubes).\n";
                    write(STDERR_FILENO, s, strlen(s));
                    result = 1;
                    goto cleanup_return;
                }

                type = *(optarg);

                break;
            
            case 'p' :
                if((max_parralel = stringToPositiveInteger(optarg)) == -1){
                    write(2, errorMessage, strlen(errorMessage));
                    result = 2;
                    goto cleanup_return;
                };

                break;

            case '?' :
                write(2, errorMessage, strlen(errorMessage));
                result = 2;
                goto cleanup_return;
        }   
    }
    int firstind = optind + 3;
    if(strcmp("{", argv[firstind]) != 0 || strcmp("}", argv[argc-1]) != 0 || isBraceGood(argv) == 0){
        write(2, errorMessage, strlen(errorMessage));
        result = 2;
        goto cleanup_return;
    }

    struct stat st;

    while((entry = readdir(dir)) != NULL){
        char *newref = getRelativeReference(argv[3], entry->d_name);
        char *ref_extension = extract_extension(entry->d_name);
        if(lstat(newref, &st) == -1){
            result = 1;
            goto cleanup_return;
        }
        if(strcmp(entry->d_name, ".") == 0 
        || strcmp(entry->d_name, "..") == 0 
        || (a_option == 0 && entry->d_name[0] == '.') 
        || (extension != NULL && !(ref_extension != NULL && strcmp(extension, ref_extension) == 0))
        || (type != '\0' && !isSameType(st.st_mode, type) && r_option == 0)
        ){
            free(newref);
            free(ref_extension);
            continue;
        }
        else{
            int temp;
            if(extension != NULL){
                free(newref);
                char *remove_extension = remove_extension_from_file(entry->d_name);
                newref = getRelativeReference(argv[3], remove_extension);
                free(remove_extension);
            }
            int bufsize = argc - (firstind) - 2;
            command *entry  = split_argv_for(bufsize, argv + firstind + 1, argc - firstind, newref, varname);
            if(type == '\0' || isSameType(st.st_mode, type)){

                
            if(commandList==NULL) commandList = newLinkedList(entry);
            else addLast(commandList, entry);
              
                
            }
            if(isSameType(st.st_mode, 'd') && r_option){
                command *newpart = copyPartition(commande);
                char **newPartList = newpart->list;
                free(newPartList[3]);

                if((newPartList[3] = malloc(strlen(newref) + 1)) == NULL){
                    perror("Erreur lors de l'allocation mémoire");
                    freeCommand(newpart);
                    result = 1;
                    goto cleanup_return;
                }

                strcpy(newPartList[3], newref);
                temp = cmd_for(newpart,commandList, parralel);

                if(temp > result) result = temp;
                freeCommand(newpart);
            }

        }
        free(newref);
        free(ref_extension);
    }

    *parralel = max_parralel;

    goto cleanup_return;

    cleanup_return :
        if(dir != NULL) closedir(dir);
        if(varname != NULL) free(varname);
        if(extension != NULL) free(extension);
        if(entry != NULL) free(entry);

    return result;
}