run:
	cd ./core
	gcc main.c error.c analisadortools.c analisadorsintatico.c analisadorlexico.c -o ../program
	cd ../