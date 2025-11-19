# Hello World

OBJ = main.o helpers.o ghost.o hunter.o

run: $(OBJ)
	gcc -Wall -o p $(OBJ)
main.o: main.c helpers.h defs.h ghost.h hunter.h
	gcc -c main.c

helpers.o: helpers.c helpers.h ghost.h hunter.h defs.h
	gcc -c helpers.c

ghost.o: ghost.c ghost.h helpers.h defs.h
	gcc -c ghost.c

hunter.o: hunter.c hunter.h helpers.h defs.h
	gcc -c hunter.c


clean: 
	rm -f $(OBJ)
	clear