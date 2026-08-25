#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "basics/basics.h"
#include "basics/error.h"
#include "analisador/tokenlexico.h"
#include "analexico.h"

Token* storedToken = NULL;

Token* readNextToken() {
    string currentIdentifier = (string) malloc(sizeof(char) * CAP_SIZE_IDENTIFIER);
    if (currentIdentifier != NULL) {
        memset(currentIdentifier, '\0', sizeof(char) * CAP_SIZE_IDENTIFIER);
    }
    else {
        return NULL;
    }

    char currentCharacter;

    while (true)
    {
        currentCharacter = (char) fgetc();
        if (currentCharacter == EOF) {
            storedToken = &tokenDefinitions[TOKEN_OPER_EOF];
            return storedToken;
        }

        sizeof(NoLista)

        if (strlen(currentIdentifier) + 1 < CAP_SIZE_IDENTIFIER) {
            strcat(currentIdentifier, currentCharacter);
        }
        else {
            return NULL;
        }


    }
}

Token* getCurrentToken() {
    return storedToken;
}
