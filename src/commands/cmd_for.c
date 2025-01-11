#include <sys/stat.h>
#include <dirent.h>

#include "../../include/command.h"
#include "../../include/structures/commandList.h"
#include "../../include/utils.h"

/**
 * Fonction permettant d'avoir une nouvelle partition de chaîne de caractère en prennant en compte le fait de remplacer $F par le nom d'une entrée donné
 * @param bufSize Taille du tableau de la chaîne de caractère.
 * @param argv Tableau de chaîne de caractère initiale.
 * @param argvsize La taille du tableau de chaîne de caractère
 * @param ref Chaîne de caractère représentant la valeur de la variable.
 * @param varname Chaîne de caractère représentant la variable.
 * @return Nouvelle partition de chaîne de caractère avec $F remplacé par la référence de l'entrée.
 */
command * split_argv_for(int bufSize, char **argv, int argvsize, char *ref, char *varname){
    char **partition = NULL;
    int i;
    if((partition = malloc((bufSize + 1) * sizeof(char *))) == NULL) goto error;
    for(i = 0; i < argvsize-2; i++){
        if(strcmp(varname, argv[i]) == 0){
            if((partition[i] = malloc(strlen(ref) + 1)) == NULL) goto error;
            strcpy(partition[i], ref);
        }
        
        else{
            char *newvar = replace_var(argv[i], varname, ref);
            if((partition[i] = malloc(strlen(newvar) + 1)) == NULL) goto error;
            strcpy(partition[i], newvar);
            free(newvar);
        };
    }
    partition[bufSize] = (char *) NULL;
    command *res = newCommandList(partition);
    return res;

    error:
        perror("Erreur lors de la copie d'une chaine de caractère");
        if(partition != NULL) free_double_buffer(partition, bufSize+1);
        exit(1);
}

/**
 * Méthode permettant d'appliquer la commande for.
 * @param commande La commande ayant appelé la méthode et contenant les arguments nécessaires si correct.
 * @param commandeList Pointeur vers une liste chaînée où sera stocké toutes les commandes à exécuter en fonction des fichiers.
 * @param parralel Valeur permettant de savoir si une approche parallèle est nécessaire. Elle vaut -1 si ce n'est pas le cas et une valeur strictement positif sinon.
 * @return 0 si la méthode s'est bien deroulé, 1 si la méthode s'est mal déroulé et 2 en cas d'erreur de syntaxe.
 */
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
                if((max_parralel = stringToPositiveInteger(optarg)) <= 0){
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
    if(strcmp("{", argv[firstind]) != 0 || strcmp("}", argv[argc-1]) != 0 || isSeparatorGood(commande, "{", "}") == 0){
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
                addLast(commandList, entry);// On ajoute la commande à la fin de la liste
            }else { 
                freeCommand(entry);
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