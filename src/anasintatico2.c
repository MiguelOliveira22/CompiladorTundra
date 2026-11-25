#include <stdio.h>
#include <string.h>

#include "analexico.h"
#include "tokenlexico.h"
#include "error.h"
#include "basics.h"

#include "symboltable.h"
#include "anasintatico.h"
#include "compiladortundra.h"

void analizeSynSem(FILE* file) {
    compilaPrograma(file, 0);
}

static void compilaPrograma(FILE* file, int escopo) {
    Token* ctk;

    writeCommand(INST_CONTROLE_INICIA_PROGRAMA, NULL, 0);

    ctk = readNextToken(file);
    if (ctk->codigo != TOKEN_KEYW_PROGRAMA) {
        sairErroTerminal(ERROR_INVALID_TOKEN, "");
    }

    ctk = readNextToken(file);
    if (ctk->codigo != TOKEN_OPER_IDENTIFICADOR) {
        sairErroTerminal(ERROR_INVALID_TOKEN, "");
    }

    // TODO: Armazenar Nome Programa Para Chamada de Procedimento Main;

    ctk = readNextToken(file);
    if (ctk->codigo != TOKEN_SYMB_ABREPARENTESES) {
        sairErroTerminal(ERROR_INVALID_TOKEN, "");
    }

    ctk = readNextToken(file);
    while (ctk->codigo != TOKEN_SYMB_FECHAPARENTESES) {
        ctk = getCurrentToken();
        if (ctk->codigo != TOKEN_OPER_IDENTIFICADOR) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "");
        }

        ctk = readNextToken(file);
        if (ctk->codigo != TOKEN_SYMB_FECHAPARENTESES && ctk->codigo != TOKEN_SYMB_VIRGULA) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "");
        }

        if (ctk->codigo == TOKEN_SYMB_VIRGULA) {
            readNextToken(file);
        }
    }

    ctk = readNextToken(file);
    if (ctk->codigo != TOKEN_SYMB_PONTOVIRGULA) {
        sairErroTerminal(ERROR_INVALID_TOKEN, "");
    }

    compilaBloco(file, escopo);

    ctk = readNextToken(file);
    if (ctk->codigo != TOKEN_SYMB_PONTO) {
        sairErroTerminal(ERROR_INVALID_TOKEN, "");
    }

    ctk = readNextToken(file);
    if (ctk->codigo != TOKEN_OPER_EOF) {
        sairErroTerminal(ERROR_INVALID_TOKEN, "");
    }

    writeCommand(INST_CONTROLE_PARA, NULL, 0);

    return;
}

static void compilaBloco(FILE* file, int escopo) {
    Token* ctk;

    ctk = readNextToken(file);

    while (true) {
        ctk = getCurrentToken();

        if (ctk->codigo == TOKEN_KEYW_ROTULO) {
            do {
                ctk = readNextToken(file);
                if (ctk->codigo != TOKEN_OPER_NUMERO) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "");
                }

                ctk = readNextToken(file);
                if (ctk->codigo != TOKEN_SYMB_VIRGULA && ctk->codigo != TOKEN_SYMB_PONTOVIRGULA) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "");
                }
            } while(ctk->codigo != TOKEN_SYMB_PONTOVIRGULA);

            readNextToken(file);
            continue;
        }

        if (ctk->codigo == TOKEN_KEYW_TIPO) {
            ctk = readNextToken(file);

            do {
                if (ctk->codigo != TOKEN_OPER_IDENTIFICADOR) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "");
                }

                ctk = readNextToken(file);
                if (ctk->codigo != TOKEN_SYMB_IGUAL) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "");
                }

                compilaTipo(file, escopo);

                ctk = readNextToken(file);
                if (ctk->codigo != TOKEN_SYMB_PONTOVIRGULA) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "");
                }

                ctk = readNextToken(file);
            } while (ctk->codigo == TOKEN_OPER_IDENTIFICADOR);

            continue;
        }

        if (ctk->codigo == TOKEN_KEYW_VARIAVEL) {
            ctk = readNextToken(file);

            do {
                if (ctk->codigo != TOKEN_OPER_IDENTIFICADOR) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "");
                }

                ctk = readNextToken(file);
                if (ctk->codigo != TOKEN_SYMB_DOISPONTOS && ctk->codigo != TOKEN_SYMB_VIRGULA) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "");
                }

                if (ctk->codigo == TOKEN_SYMB_VIRGULA) {
                    ctk = readNextToken(file);
                    continue;
                }

                ctk = readNextToken(file);
                if (ctk->codigo != TOKEN_OPER_IDENTIFICADOR) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "");
                }

                ctk = readNextToken(file);
                if (ctk->codigo != TOKEN_SYMB_PONTOVIRGULA) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "");
                }

                ctk = readNextToken(file);
            } while (ctk->codigo == TOKEN_OPER_IDENTIFICADOR);

            continue;
        }

        if (ctk->codigo == TOKEN_KEYW_PROCEDIMENTO) {
            ctk = readNextToken(file);
            if (ctk->codigo != TOKEN_OPER_IDENTIFICADOR) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "");
            }

            ctk = readNextToken(file);
            if (ctk->codigo == TOKEN_SYMB_ABREPARENTESES) {
                compilaParametrosFormais(file, escopo);
            }

            ctk = readNextToken(file);
            if (ctk->codigo != TOKEN_SYMB_PONTOVIRGULA) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "");
            }

            compilaBloco(file, escopo + 1);

            ctk = readNextToken(file);
            if (ctk->codigo != TOKEN_SYMB_PONTOVIRGULA) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "");
            }

            readNextToken(file);
            continue;
        }

        if (ctk->codigo == TOKEN_KEYW_FUNCAO) {
            ctk = readNextToken(file);
            if (ctk->codigo != TOKEN_OPER_IDENTIFICADOR) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "");
            }

            ctk = readNextToken(file);
            if (ctk->codigo == TOKEN_SYMB_ABREPARENTESES) {
                compilaParametrosFormais(file, escopo);
            }

            ctk = getCurrentToken();
            if (ctk->codigo != TOKEN_SYMB_DOISPONTOS) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "");
            }

            ctk = readNextToken(file);
            if (ctk->codigo != TOKEN_OPER_IDENTIFICADOR) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "");
            }

            ctk = readNextToken(file);
            if (ctk->codigo != TOKEN_SYMB_PONTOVIRGULA) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "");
            }

            compilaBloco(file, escopo + 1);

            ctk = readNextToken(file);
            if (ctk->codigo != TOKEN_SYMB_PONTOVIRGULA) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "");
            }

            readNextToken(file);
            continue;
        }

        if (ctk->codigo == TOKEN_KEYW_INICIO) {
            readNextToken(file);

            do {
                compilaComando();

                ctk = getCurrentToken();
            } while (ctk->codigo != TOKEN_KEYW_FIM);

            writeCommand(INST_CONTROLE_DELOCA_MEM, ); // TODO: Descrever o Dealoc da Memória

            return;
        }

        sairErroTerminal(ERROR_INVALID_TOKEN, "");
    }
}

// Retorna a Lista de Parametros
static Lista compilaParametrosFormais(FILE* file, int escopo) {
    Token* ctk;

    while (true) {
        ctk = getCurrentToken();
        if (ctk->codigo == TOKEN_SYMB_FECHAPARENTESES) {
            return; // TODO: Retornar Lista
        }

        ctk = readNextToken(file);

        if (ctk->codigo == TOKEN_KEYW_VARIAVEL || ctk->codigo == TOKEN_OPER_IDENTIFICADOR) {
            if (ctk->codigo == TOKEN_KEYW_VARIAVEL) {
                readNextToken(file);
            }

            do {
                ctk = getCurrentToken();
                if (ctk->codigo != TOKEN_OPER_IDENTIFICADOR) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "");
                }

                // Symtab

                ctk = readNextToken(file);
                if (ctk->codigo != TOKEN_SYMB_VIRGULA && ctk->codigo != TOKEN_SYMB_DOISPONTOS) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "");
                }

                readNextToken(file);
            } while(ctk->codigo != TOKEN_SYMB_DOISPONTOS);

            compilaTipo();

            ctk = readNextToken(file);
            if (ctk->codigo != TOKEN_SYMB_PONTOVIRGULA && ctk->codigo != TOKEN_SYMB_FECHAPARENTESES) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "");
            }

            continue;
        }

        if (ctk->codigo == TOKEN_KEYW_FUNCAO) {
            do {
                ctk = readNextToken(file);
                if (ctk->codigo != TOKEN_OPER_IDENTIFICADOR) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "");
                }

                // Symtab

                ctk = readNextToken(file);
                if (ctk->codigo != TOKEN_SYMB_VIRGULA && ctk->codigo != TOKEN_SYMB_DOISPONTOS) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "");
                }
            } while(ctk->codigo != TOKEN_SYMB_DOISPONTOS);

            compilaTipo();

            ctk = readNextToken(file);
            if (ctk->codigo != TOKEN_SYMB_PONTOVIRGULA && ctk->codigo != TOKEN_SYMB_FECHAPARENTESES) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "");
            }

            continue;
        }

        if (ctk->codigo == TOKEN_KEYW_PROCEDIMENTO) {
            do {
                ctk = readNextToken(file);
                if (ctk->codigo != TOKEN_OPER_IDENTIFICADOR) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "");
                }

                // Symtab

                ctk = readNextToken(file);

                if (ctk->codigo == TOKEN_SYMB_VIRGULA) {
                    continue;
                }

                if (ctk->codigo != TOKEN_SYMB_PONTOVIRGULA && ctk->codigo != TOKEN_SYMB_FECHAPARENTESES) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "");
                }
            } while(ctk->codigo != TOKEN_SYMB_PONTOVIRGULA && ctk->codigo != TOKEN_SYMB_FECHAPARENTESES);

            continue;
        }

        sairErroTerminal(ERROR_INVALID_TOKEN, "");
    }
}

static void compilaTipo(FILE* file, int escopo) {
    Token* ctk;

    ctk = readNextToken(file);
    if (ctk->codigo == TOKEN_OPER_IDENTIFICADOR) {
        return tipoValido;
    }

    sairErroTerminal(ERROR_INVALID_TOKEN, "");
}

static void compilaComando(FILE* file, int escopo) {
    Token* ctk;

    ctk = readNextToken(file);
    if (ctk->codigo == TOKEN_OPER_NUMERO) {
        // Adicionar Label No Escopo

        ctk = readNextToken(file);
        if (ctk->codigo != TOKEN_SYMB_DOISPONTOS) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "");
        }

        readNextToken(file);
    }

    compilaComandoSemRotulo();
}

static void compilaComandoSemRotulo() {
    Token* ctk;

    ctk = getCurrentToken();

    if (ctk->codigo == TOKEN_OPER_IDENTIFICADOR) {

    }

    if (ctk->codigo == TOKEN_KEYW_VAPARA) {

    }

    if (ctk->codigo == TOKEN_KEYW_INICIO) {

    }

    if (ctk->codigo == TOKEN_KEYW_SE) {

    }

    if (ctk->codigo == TOKEN_KEYW_ENQUANTO) {

    }

    sairErroTerminal(ERROR_INVALID_TOKEN, "");
}

static void compilaExpressao() {

}

static void compilaTermo() {

}

static void compilaFator() {

}
