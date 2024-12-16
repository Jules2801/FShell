# Commande de compilation par défaut
CC := gcc
CFLAGS := -o fsh
SRC := fsh.c utils.c commandList.c redirection.c command.c prompt.c
LDFLAGS := -lreadline

# Cible principale
all: fsh

fsh:
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS)

# Nettoyage
clean:
	rm -f fsh
