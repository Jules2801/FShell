#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

/**
 * Structure représentant une commande
 * Contient un tableau de chaînes de caractères (list) et un entier (size) indiquant sa taille
 */
typedef struct command {
    char **list;  // Liste des parties de la commande
    int size;     // Nombre d'éléments dans la liste
} command;


/**
 * Structure représentant un noeud dans la liste chaînée
 * Chaque noeud contient une donnée de type `command` et un pointeur vers le noeud suivant
 */
typedef struct node {
    command *data;  // Pointeur vers la commande
    struct node *next;  // Pointeur vers le noeud suivant dans la liste
} node;


/**
 * Structure représentant une liste chaînée de commandes
 * Contient un pointeur vers le premier noeud de la liste
 */
typedef struct linkedList {
    node* head;  // Premier noeud de la liste chaînée
} linkedList;


/**
 * Fonction permettant de créer une structure `command`
 * @param fullcommand Chaîne de caractères représentant la commande
 * @return Un pointeur vers une structure `command` ou NULL en cas d'erreur d'allocation mémoire
 **/
command *newCommand(char *fullcommand);

/**
 * Fonction permettant de créer une structure `command`
 * @param fullcommand liste de chaîne de caractères représentant la commande (partition)
 * @return Un pointeur vers une structure `command` ou NULL en cas d'erreur d'allocation mémoire
 **/
command *newCommandList(char **fullcommands);

/**
 * Fonction permettant de libérer la mémoire d'une structure `command`
 * @param command La structure `command` à libérer
 **/
void freeCommand(command *command);

/**
 * Fonction permettant de créer une liste chaînée vide
 * @return Une liste chaînée ou NULL si problème d'allocation mémoire
 **/
linkedList *newEmptyLinkedList();

/**
 * Fonction permettant de créer une nouvelle liste chaînée
 * @param head Pointeur vers la première commande de la liste
 * @return Un pointeur vers une nouvelle structure `linkedList` ou NULL en cas d'erreur d'allocation mémoire
 **/
linkedList *newLinkedList(command *head);


/**
 * Fonction permettant d'ajouter une commande au début de la liste chaînée
 * @param list Pointeur vers la liste chaînée
 * @param data Pointeur vers la commande à ajouter
 **/
void addFirst(linkedList *list, command *data);


/**
 * Fonction permettant d'ajouter une commande à la fin de la liste chaînée
 * @param list Pointeur vers la liste chaînée
 * @param data Pointeur vers la commande à ajouter
 **/
void addLast(linkedList *list, command *data);

/**
 * Fonction permettant d'itérer sur la liste chaînée en supprimant le premier élément
 * @param list Pointeur vers la liste chaînée
 * @return Un pointeur vers la liste chaînée mise à jour ou NULL si la liste est vide
 **/
linkedList *itere(linkedList *list);

/**
 * Fonction permettant de récupérer la commande du premier noeud de la liste
 * @param list Pointeur vers la liste chaînée
 * @return Un pointeur vers la commande du premier noeud ou NULL si la liste est vide
 **/
command *getCommand(linkedList *list);

/**
 * Fonction permettant de libérer récursivement tous les noeuds parents
 * @param noeud Le noeud à partir duquel commencer la libération
 **/
void freeAllNodes(node *noeud);

/**
 * Fonction permettant de libérer la mémoire d'un noeud
 * @param noeud Le noeud à libérer
 **/
void freeNode(node *noeud);


/**
 * Fonction permettant de libérer la mémoire d'une liste chaînée
 * @param list La liste chaînée à libérer
 **/
void freeLinkedList(linkedList *list);

#endif  
