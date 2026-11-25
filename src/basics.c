#include <stdlib.h>

#include "basics.h"

string strduplic(string data) {
    if (data == NULL) {
        return NULL;
    }

    int size = (strlen(data) + 1) * sizeof(char);
    string novo = (string) malloc(size);

    if (novo == NULL) {
        return NULL;
    }

    memcpy(novo, data, size);
    return novo;
}