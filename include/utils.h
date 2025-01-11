#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "structures/commandList.h"

/**
 * Fonction permettant de compter le nombre de mots dans une chaîne de caractères.
 * @param command Une chaîne de caractères.
 * @return Un entier représentant le nombre de mots dans la chaîne de caractères.
 */
int count_words(char *command);

/**
 * Fonction permettant de couper une chaîne de caractères en un tableau de chaînes de caractères.
 * @param command Une chaîne de caractères.
 * @return Un tableau de chaînes de caractères avec NULL en dernière position.
 */
char **split_command(char *command);

/**
 * Fonction permettant de compter le nombre d'éléments d'une partition de chaînes de caractères.
 * @param argv Partition de chaînes de caractères.
 * @return La taille de la partition.
 */
int partSize(char **argv);

/**
 * Fonction permettant d'obtenir l'emplacement relatif d'une entrée depuis un répertoire donné.
 * @param dirname Le chemin relatif du répertoire actuel.
 * @param filename Le nom de l'entrée.
 * @return Une chaîne de caractères représentant l'emplacement relatif.
 */
char *getRelativeReference(char *dirname, char *filename);

/**
 * Fonction permettant de libérer un buffer représentant un tableau de chaînes de caractères.
 * @param buffer Le tableau de chaînes de caractères.
 * @param bufsize La taille du buffer.
 */
void free_double_buffer(char **buffer, int bufsize);

/**
 * Fonction permettant de vérifier qu'une chaîne de caractères est égale à une autre sur les n premiers octets.
 * @param str Chaîne de caractères à vérifier.
 * @param var Chaîne de caractères de référence.
 * @return 1 si elles sont égales, 0 sinon.
 */
int isEqualFirst(const char *str, const char *var);

/**
 * Fonction permettant de remplacer une variable par une valeur dans une chaîne de caractères.
 * @param arg La chaîne de caractères d'origine.
 * @param var La variable à remplacer.
 * @param name La valeur de remplacement.
 * @return Une nouvelle chaîne de caractères avec la variable remplacée.
 */
char *replace_var(char *arg, char *var, char *name);

/**
 * Fonction permettant de vérifier si une chaîne de caractères représente un entier positif.
 * @param arg La chaîne de caractères.
 * @return 1 si c'est un entier positif, 0 sinon.
 */
int isPositiveInteger(char *arg);

/**
 * Fonction permettant de convertir une chaîne de caractères en entier positif.
 * @param arg La chaîne de caractères.
 * @return L'entier représenté par la chaîne ou -1 en cas d'échec.
 */
int stringToPositiveInteger(char *arg);

/**
 * Fonction permettant d'extraire l'extension d'une référence donnée.
 * @param arg La référence.
 * @return L'extension si elle existe, NULL sinon.
 */
char *extract_extension(char *arg);

/**
 * Fonction permettant de vérifier si des spérateurs dans une partition sont équilibrées.
 * @param commande La commande.
 * @param open_separator Le séparateur ouvrant.
 * @param close_separator Le séparateur fermant.
 * @return 1 si équilibrées, 0 sinon.
 */
int isSeparatorGood(command *commande, char *open_separator, char *close_separator);

/**
 * Méthode permettant de vérifier si la chaîne de caractère fourni ne représente pas une valeur interdites.
 * @param argv La chaine de caractère.
 * @return 1 si c'est le cas et 0 sinon.
 */
int isIllegalName(char *argv);

/**
 * Fonction permettant de retirer l'extension d'un fichier.
 * @param arg Le nom du fichier.
 * @return Le nom sans extension.
 */
char *remove_extension_from_file(const char *arg);

/**
 * Fonction permettant de concaténer deux chaînes de caractères avec un saut de ligne.
 * @param arg1 Première chaîne de caractères.
 * @param arg2 Deuxième chaîne de caractères.
 * @return Une chaîne de caractères concaténée.
 */
char *concat(char *arg1, char *arg2);

/**
 * Fonction permettant de vérifier si une partition correspond à une commande structurée.
 * @param command La commande.
 * @return 1 si structurée, 0 sinon, -1 si invalide.
 */
int isStructuredCommand(command *command,char *separator);
 
/**
 * Fonction permettant de compter le nombre de commandes dans une commande structurée.
 * @param command La commande structurée.
 * @return Le nombre de commandes.
 */
int nbCommand(command *command);

/**
 * Fonction permettant de créer une sous-partition d'une partition donnée.
 * @param commande La partition de base.
 * @param first Position de début.
 * @param last Position de fin.
 * @return La sous-partition.
 */
command *makeSubPartition(command *commande, int first, int last);

/**
 * Fonction permettant de créer une liste de partitions pour une commande structurée.
 * @param commande La commande structurée.
 * @param size Pointeur pour stocker la taille de la liste.
 * @return La liste de partitions.
 */
command **makeStructuredCommand(command *commande, int *size,char *separator);

/**
 * Fonction permettant de vérifier si un type de fichier correspond au caractère donné.
 * @param mode Le mode du fichier.
 * @param f Le caractère du type de fichier ('f', 'd', 'l', 'p').
 * @return 1 si correspond, 0 sinon.
 */
int isSameType(mode_t mode, char f);

/**
 * Fonction permettant de copier une partition de chaînes de caractères.
 * @param commande La partition à copier.
 * @return Une copie de la partition.
 */
command *copyPartition(command *commande);

/**
 * Fonction qui stocke la position d'un "else" das une commande structurée "if" s'il existe.
 * @param commande La commande structurée.
 * @param else_pos pointeur vers une variable de type int.
 */
void hasElse(char **commande, int *else_pos);


#endif