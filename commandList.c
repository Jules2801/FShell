#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include "utils.h"
#include "commandList.h"


/**
 * Fonction permettant de créer une struct command
 * @param fullcommand Chaîne de caractère
 * @return Une struct commande ou NULL si erreur d'allocation mémoire
 **/
command *newCommand(char *fullcommand){
    command *newCommand = malloc(sizeof(command));
    if (!newCommand) {
    write(2, "Erreur de d'allocation d'une structure commande\n", 49);
    return NULL;
    }

    char **newlist = split_command(fullcommand);

    if (!newlist) {
        free(newCommand);
        write(2, "Erreur de d'allocation d'une liste de commande\n", 46);
        return NULL;
    }

    newCommand->list = newlist;
    int size = partSize(newlist);
    if (size <= 0) { // Gérer le cas où partSize échoue (si applicable)
        free(newlist);
        free(newCommand);
        write(2, "Erreur lors du calcul de la taille de la commande\n", 50);
        return NULL;
    }
    return newCommand;
}

command *newCommandList(char **fullcommands){
    command *newCommand = malloc(sizeof(command));
    if (!newCommand) {
    write(2, "Erreur de d'allocation d'une structure commande\n", 49);
    return NULL;
    }


    newCommand->list = fullcommands;
    int size = partSize(fullcommands);
       if (size <= 0) { // Gérer le cas où partSize échoue (si applicable)
        free(fullcommands);
        free(newCommand);
        write(2, "Erreur lors du calcul de la taille de la commande\n", 50);
        return NULL;
    }

    return newCommand;
}

/**
 * Fonction permettant de libérer la mémoire d'une struct command
 * @param command La struct à libérer
 **/
void freeCommand(command *command){
    if (!command) {
        return; //Dans le cas où command est NULL, on ne fait rien
    }
    free_double_buffer(command->list, command->size);
    free(command);
}




/**
 * Fonction permettant de créer une liste chaînée 
 * @param head une struct command
 * @return Une liste chaînée ou NULL si problème d'allocation mémoire
 **/
linkedList *newLinkedList(command *head){
    linkedList *newlist = malloc(sizeof(linkedList));
    if(!newlist){
        write(2, "Erreur d'allocation d'une liste chaînée\n", 41);
        return NULL;
    }
    node *newNode = malloc(sizeof(node));
    if (!newNode) {
        write(2, "Erreur d'allocation d'un noeud\n", 31);
        free(newlist);
        return NULL;
    }
    newNode->data = head;
    newNode->next = NULL;
    newlist -> head = newNode;
    
    return newlist;

}

/**
 * Fonction permettant d'ajouter au début de la liste chainée
 * @param list la liste chaînée
 * @param data la struct commande à ajouter 
 **/
void addFirst(linkedList *list, command *data){
    node *newNode = malloc(sizeof(node));

    if (!newNode) {
        write(2, "Erreur d'allocation d'un noeud\n", 31);
        return;
    }

    newNode->data = data;
    newNode->next = list->head;
    list->head = newNode;
}

/**
 * Fonction permettant d'ajouter à la fin de la liste chaînée
 * @param list la liste chaînée
 * @param data la struct commande à ajouter 
 **/
void addLast(linkedList *list, command *data){
    node* newNode = (node*) malloc(sizeof(node));

    if (!newNode) {
        write(2, "Erreur d'allocation d'un noeud\n", 31);
        return;
    }
    
    newNode->data = data;
    newNode->next = NULL;

    if (!list->head) {
        list->head = newNode;
        
    }
    else{

    node* temp = list->head;
    while (temp->next != NULL) {
        temp = temp->next;
    }

    temp->next = newNode;
    }
}

/**
*Fonction permettant de supprimer le premier noeud de la liste chainé
*@param list la liste chaînée
*@return la liste chainée après suppression du premier noeud ou NULL si la liste est vide
 */
linkedList *itere(linkedList *list){
    if(!list||!(list->head)){
        return NULL;
    }
    node *tmp = list->head->next;
    freeNode(list->head);
    list->head = tmp;
    return list;
}

/**
*Fonction de recuperer la commande du premier noeud de la liste chainée
*@param list la liste chaînée
*@return la commande du premier noeud de la liste chainée ou NULL si la liste est vide
 */
command *getCommand(linkedList *list){
    if(!list||!(list->head)){
        return NULL;
    }
    return list->head->data;
}

/**
 * Fonction permettant de libérer un seul noeud
 * @param noeud le noeud à libérer  
 **/
void freeNode(node *noeud) {
    if (!noeud) return;
    freeCommand(noeud->data); 
    noeud->next = NULL;
    free(noeud);             
}


/**
 * Fonction permettant de libérer récursivement tous les noeuds parents
 * @param noeud le noeud à libérer  
 **/
void freeAllNodes(node *noeud) {
    
    if (!noeud) return;
    
    freeCommand(noeud->data);

    if (noeud->next) freeAllNodes(noeud->next);
    
    free(noeud);
}

/**
 * Fonction permettant de libérer la mémoire d'une liste chaînée
 * @param list la liste chaînée à libérer
 **/
void freeLinkedList(linkedList *list){
    if(!list){
        return;
    }
    freeAllNodes(list->head);
    free(list);
 
}
