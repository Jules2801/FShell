# Commande de compilation par défaut
CC := gcc
CFLAGS := -o fsh
SRC := src/fsh.c src/utils.c src/structures/commandList.c src/structures/redirection.c src/command.c src/prompt.c src/commands/cmd_exit.c src/commands/cmd_cd.c src/commands/cmd_pwd.c src/commands/cmd_ftype.c src/commands/cmd_if.c src/commands/cmd_for.c
LDFLAGS := -lreadline

# Cible principale
all: fsh

fsh:
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS)

# Nettoyage
clean:
	rm -f fsh
