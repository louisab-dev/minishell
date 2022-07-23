# Définition des Variables (à compléter)
CC = gcc
CFLAGS = -Wall -c
LDFLAGS = -o
EXE = minishell

# Règles (à compléter avec des variables automatiques)
all: $(EXE)

minishell: job.o readcmd.o minishell.o
	$(CC) $(LFLAGS) $^ -o $@

depend: 
	makedepend *.c -Y.

clean:
	rm -rf *.o $(EXE)

.PHONY: clean all depend
# DO NOT DELETE

job.o: job.h
minishell.o: readcmd.h job.h
minishell2.o: readcmd.h
minishell6.o: readcmd.h job.h
readcmd.o: readcmd.h
