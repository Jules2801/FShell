# Projet `fsh`

## Description
`fsh` est un interpréteur de commandes interactif écrit en C, conçu pour reproduire des fonctionnalités classiques des shells tout en introduisant des commandes spécifiques comme des boucles `for` avancées.

## Fonctionnalités principales
- **Commandes internes** : `pwd`, `cd`, `exit`, `ftype`.
- **Redirections** : Gestion complète des redirections (`<`, `>`, `>>`, etc.).
- **Pipelines** : Support des combinaisons par tube (`|`).
- **Commandes structurées** : `if`, `for`, avec options avancées (`-A`, `-r`, `-e`, `-t`, `-p`).
- **Gestion des signaux** : Interruption (`SIGINT`), ignorance de certains signaux (`SIGTERM`).
- **Prompt dynamique** : Affiche le répertoire courant, l'état de la dernière commande, et des indicateurs colorés.

## Compilation et exécution
- **Compiler** :
  ```bash
  make
  ```
- **Lancer** :
  ```bash
  ./fsh
  ```
- **Nettoyer** :
  ```bash
  make clean
  ```

## Structure du projet
- **Code source** : Situé dans le dossier `src`.
- **Headers** : Situés dans le dossier `include`.
- **Documentation** : Disponible dans `ARCHITECTURE.md`.

## Auteurs
Liste des contributeurs dans le fichier `AUTHORS.md`. 