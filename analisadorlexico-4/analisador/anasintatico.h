#ifndef anasintatico
#define anasintatico

#include <stdlib.h>

void compilaPrograma(FILE* file, int escopo);
void compilaBloco(FILE* file, int escopo);
void compilaParametrosFormais(FILE* file, int escopo);
void compilaComando(FILE* file, int escopo);
void compilaComandoSemRotulo(FILE* file, int escopo);
void compilaExpressao(FILE* file, int escopo);
void compilaExpressaoSimples(FILE* file, int escopo);
void compilaTermo(FILE* file, int escopo);
void compilaFator(FILE* file, int escopo);

void anasin(FILE* file);

#endif