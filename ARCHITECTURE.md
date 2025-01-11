# ARCHITECTURE.md

## Introduction
Ce document explique l'architecture logicielle et les choix de conception adoptés pour le projet `fsh`, un interpréteur de commandes interactif avec des fonctionnalités avancées comme les boucles `for`, les redirections, et les commandes structurées.

---

## Vue d'ensemble
L'architecture de `fsh` repose sur une approche modulaire permettant de séparer les responsabilités en différents composants. Voici les principales composantes :

1. **Gestion de l'entrée utilisateur** : Lecture et traitement de la ligne de commande avec `readline`.
2. **Analyseur syntaxique** : Interprétation et validation des commandes saisies.
3. **Exécution des commandes** : Gestion des commandes internes, externes, pipelines, redirections et boucles.
4. **Gestion des signaux** : Traitement des interruptions et signaux spécifiques comme `SIGINT` et `SIGTERM`.
5. **Affichage du prompt** : Construction et affichage d'un prompt adaptatif.

---

## Architecture logicielle

### 1. Gestion de l'entrée utilisateur
Le module d'entrée utilise la bibliothèque `readline` pour :
- Lire les lignes de commande utilisateur.
- Gérer l'historique des commandes.
- Traiter les corrections et erreurs pendant la saisie.

**Points clés :**
- Le prompt est affiché sur `stderr`.
- Chaque ligne est nettoyée avant d'être transmise à l'analyseur syntaxique.

### 2. Analyseur syntaxique
Le rôle de l'analyseur est de décomposer la ligne de commande en ses éléments constitutifs :
- Commandes simples.
- Pipelines.
- Commandes structurées (`if`, `for`, etc.).

L'analyseur :
- Valide la syntaxe en détectant les erreurs (comme des accolades mal appariées).
- Gère les caractères spéciaux (`<`, `>`, `|`, `{`, `}`, `;`, `$`).
- Retourne une structure de données représentant la commande.

**Implémentation :**
- Utilisation d'un arbre syntaxique pour modéliser les commandes imbriquées.
- Gestion des erreurs de syntaxe avec des messages explicites et un code de retour `2`.

### 3. Exécution des commandes
Le module d'exécution gère les différents types de commandes :

#### Commandes internes
- `pwd`, `cd`, `exit`, `ftype`.
- Implémentées comme des fonctions C appelées directement.

#### Commandes externes
- Exécutées via des appels système comme `fork`, `execvp` et `waitpid`.
- Recherche dans le chemin spécifié par la variable `PATH`.

#### Redirections
- Implémentées en utilisant les appels système `dup2` pour rediriger les descripteurs de fichier.
- Validation des fichiers et gestion des erreurs (échec si fichier inexistant ou déjà présent).

#### Pipelines
- Création de tubes anonymes avec `pipe`.
- Une généalogie avec *n* fils supervisés par leur père. Avec *n* le nombre de commande dans la pipeline.

#### Commandes structurées
- Gestion des constructions comme `if` et `for`.
- Boucles `for` avec options (`-A`, `-r`, `-e`, `-t`, `-p`) gérées par`getopt()` de la librairie `unistd`.
- Implémentation des options comme des filtres appliqués sur les fichiers de la boucle.

**Optimisations :**
- Gestion parallèle des boucles avec `-p` grâce à des processus fils au nombre limité par la valeur fourni.

### 4. Gestion des signaux
- Ignorer `SIGTERM` pour le processus `fsh`.
- Capture de `SIGINT` pour interrompre les commandes en cours en remettant par défaut le gestionnaire de `SIGINT` pour les processus fils et arrêt du programme si reçu.
- Affichage de `[SIG]` dans le prompt en cas d'interruption.

**Comportement :**
- Attendre la terminaison des processus en cours avant d'afficher le prompt.
- Valeur de retour par défaut `255` si une commande est interrompue.
- Lorsqu'une commande s'est terminé par un signal, alors ce signal `SIG` est stockés dans la variable globale `erreur` dans le format `-SIG` (Donc les signaux sont représenté par des valeurs négatifs).

### 5. Affichage du prompt
Le prompt est construit dynamiquement pour refléter l'état de la dernière commande exécutée :
- **Couleur** : Vert pour succès, rouge pour échec.
- **Valeur de retour** : `[0]`, `[1]`, ou `[SIG]`.
- **Répertoire courant** : Troncation à 30 caractères avec des ellipses si nécessaire.

---

## Structures de données

### Variables globales/utiles :

- previous_directory : Stocke la reférence de l'ancien répertoire lorsque la commande cd a été utilisé au moins une fois.
- current_directory : Stocke la reférence actuel du répertoire courant.
- isInRed : Entier indiquant si la commande actuelle possède une redirection vers l'entrée standard.
- isOutRed : Entier indiquant si la commande actuelle possède une redirection vers la sortie standard.
- isErrRed : Entier indiquant si la commande actuelle possède une redirection vers la sortie erreur.
- error : Entier représentant la valeur de retour d'une commande. Elle est négative si la commande s'est terminé à cause d'un signal.

### Commande
```c
typedef struct command {
    char **list;  // Liste des parties de la commande
    int size;     // Nombre d'éléments dans la liste
} command;
```

```c
typedef struct node {
    command *data;  // Pointeur vers la commande
    struct node *next;  // Pointeur vers le noeud suivant dans la liste
} node;
```

```c
typedef struct linkedList {
    node* head;  // Premier noeud de la liste chaînée
} linkedList;
```

### Redirection
```c
typedef enum {
    RSTDIN,
    RSTDOUT,
    RSTDOUT_WITH_CRUSHING,
    RSTDOUT_WITH_APPEND,
    RSTDERR,
    RSTDERR_WITH_CRUSHING,
    RSTDERR_WITH_APPEND
} Redtype;
```

```c
typedef struct {
    Redtype type;
    char *filename;
} Redirection;
```
---

## Algorithmes

### Boucle `for`

1. Vérifier la syntaxe et les options de la commande `for`.
2. Parcourir le répertoire cible en appliquant les filtres (`-A`, `-r`, `-e`, `-t`).
3. Générer des commandes pour chaque entrée valide et les ajouter à `commandList`.
4. Gérer récursivement les sous-répertoires si l'option `-r` est activée.
5. Si l'option -p est fourni avec la valeur n, alors applique la parralélisation avec n fils.
6. Sinon, applique les commandes sur les entrées donnés un par un.

### Commande `if`
1. Vérifier le nombre d'arguments. Si insuffisant, retourner une erreur de syntaxe (`2`).
2. Localiser la première `{` et valider la syntaxe générale (appariement des séparateurs `[`, `]`, `{`, `}`).
3. Identifier la position d'un éventuel `else` et valider sa syntaxe.
4. Exécuter la commande de test `TEST` :
   - Si `TEST` réussit, exécuter la commande entre `{}` associée.
   - Sinon, exécuter la commande associée au `else`, si présent.
5. Libérer les ressources et retourner le résultat de la commande exécutée.

### Gestion des redirections
1. Vérifier la syntaxe et la validité des fichiers.
2. Utiliser `open` avec les options appropriées (`O_CREAT`, `O_APPEND`, etc.).
3. Rediriger les descripteurs avec `dup2`.

### Exécution des pipelines
1. Créer un tube pour chaque paire de commandes adjacentes.
2. Utiliser `fork` pour exécuter chaque commande dans un processus fils.
3. Connecter les sorties et entrées des tubes avec `dup2`.

### Boucles parallèles
1. Limiter le nombre de processus actifs avec la valeur fourni.
2. Créer des processus fils lorsque nécessaires.
3. Se termine lorsque toutes les commandes ont été exécutés (sauf exceptions).
