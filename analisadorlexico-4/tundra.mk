run:
	gcc main.c basics/error.c estruturas/lista.c estruturas/hashtable.c \
	analisador/analexico.c analisador/anasintatico.c \
	analisador/symboltable.c analisador/tokenlexico.c \
	-Ianalisador -Ibasics -Iestruturas -o ./tundra -std=c99
