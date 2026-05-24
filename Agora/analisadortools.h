#ifndef analisadortools
#define analisadortools

#include "basics.h"

extern const i8 MAX_LENGTH_LEXICO;

typedef enum {
    programa,
    rotulo,
    tipo,
    variavel,
    procedimento,
    funcao,
    inicio,
    fim,
    atribuicao,
    se,
    entao,
    senao,
    enquanto,
    faca,
    vapara,
    mais,
    menos,
    vezes,
    dividir,
    igual,
    diferente,
    menor,
    menorouigual,
    maior,
    maiorouigual,
    e,
    ou,
    nao,
    abreparenteses,
    fechaparenteses,
    abrecolchetes,
    fechacolchetes,
    virgula,
    pontoevirgula,
    doispontos,
    ponto,
    abrecomentario,
    fechacomentario,
    
    invalido,
    eof,
    
    null,
    identificador,
    numero,
} CodigoToken;

typedef struct {
    CodigoToken codigoToken;
    char*       lexicoArquivo;
    bool        isEspecial;
    char*       nomeCompilador;
} token;

extern const token tokens[];

token analex(FILE* file, bool updateTokens);
void  anasin(FILE* file);

#endif