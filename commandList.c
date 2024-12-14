#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/_types/_null.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <dirent.h>
#include "utils.h"



typedef struct command{
    
    char **list ;
    
    int size;

}command;

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
    if (size < 0) { // Gérer le cas où partSize échoue (si applicable)
        free(newlist);
        free(newCommand);
        write(2, "Erreur lors du calcul de la taille de la commande\n", 50);
        return NULL;
    }
    return newCommand;
}

void freeCommand(command *command){
    if (!command) {
        return; //Dans le cas où command est NULL, on ne fait rien
    }
    free_double_buffer(command->list, command->size);
    free(command);
}

typedef struct node {

    command *data;

    struct node *next;

}node;



typedef struct linkedList {

    node* head;

}linkedList;


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


void freeNode(node *noeud) {
    
    if (!noeud) return;
    
    freeCommand(noeud->data);

    if (noeud->next) freeNode(noeud->next);
    

    free(noeud);
}


void freeLinkedList(linkedList *list){
    if(!list){
        return;
    }
    freeNode(list->head);
 
}
