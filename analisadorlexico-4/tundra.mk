run:
	gcc main.c basics/error.c basics/lista.c basics/hashtable.c \
	analisador/analexico.c analisador/anasintatico.c \
	analisador/symboltable.c analisador/tokenlexico.c \
	-o ./tundra -std=c99
