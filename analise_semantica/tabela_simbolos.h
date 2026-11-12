#ifndef TABELA_SIMBOLOS_H
#define TABELA_SIMBOLOS_H

#include <stdbool.h>

// Tamanho da Tabela
#define TAMANHO_TABELA 211

// Nó para tratar colisões na Hash Table
typedef struct NoHash {
    char *nome_variavel;
    struct NoHash *prox;
} NoHash;

// A Tabela de Símbolos
typedef struct {
    NoHash *entradas[TAMANHO_TABELA];
} TabelaSimbolos;

// Funções base (assinaturas)
TabelaSimbolos* criaTabelaSimbolos();
void liberaTabelaSimbolos(TabelaSimbolos *tabela);

// Insere uma nova variável na tabela
void inserirTabela(TabelaSimbolos *tabela, const char *nome);

// Retorna true se a variável já existe, false caso contrário
bool buscarTabela(TabelaSimbolos *tabela, const char *nome);

#endif