#include <stdlib.h>
#include <string.h>
#include "tabela_simbolos.h"

// Função Hash djb2
static unsigned int calcularHash(const char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    return hash % TAMANHO_TABELA;
}

// Inicializa a tabela preenchendo com NULL
TabelaSimbolos* criaTabelaSimbolos() {
    TabelaSimbolos *tabela = malloc(sizeof(TabelaSimbolos));
    for (int i = 0; i < TAMANHO_TABELA; i++) {
        tabela->entradas[i] = NULL;
    }
    return tabela;
}