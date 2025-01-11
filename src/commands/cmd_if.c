#include "../../include/command.h"
#include "../../include/utils.h"

/**
 * Méthode permettant d'appliquer la commande if.
 * @param commande La commande ayant appelé la méthode et contenant les arguments nécessaires si correct.
 * @return 0 si la méthode s'est bien deroulé, 1 si la méthode s'est mal déroulé et 2 en cas d'erreur de syntaxe.
 */
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

    if(strcmp("}", argv[argc-1]) != 0 || strcmp("{", argv[first_opening_bracket_pos]) != 0 || isSeparatorGood(commande, "{", "}") == 0 || isSeparatorGood(commande, "[", "]") == 0){
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