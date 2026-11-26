#ifndef TABELA_SIMBOLOS_H
#define TABELA_SIMBOLOS_H

#include <stdbool.h>

#define TAMANHO_TABELA 211

typedef enum {
    SIMB_VAR_GLOBAL,
    SIMB_VAR_LOCAL,  // Inclui variáveis locais e parâmetros formais
    SIMB_FUNCAO
} TipoSimbolo;

typedef struct TabelaSimbolos TabelaSimbolos;

/*
 * Convenção de offsets em relação a %rbp para SIMB_VAR_LOCAL (com L variáveis locais):
 * - Variáveis locais (índice i = 0, ..., L-1): offset = i * 8
 *     Ex: 0(%rbp), 8(%rbp), ...
 * - Parâmetros formais (índice j = 0, ..., P-1, empilhados em ordem inversa):
 *     offset = (L + 2 + j) * 8  (pula L locais + %rbp salvo + endereço de retorno)
 */
typedef struct NoHash {
    char *nome;
    TipoSimbolo tipo;
    int offset;                    // Usado quando tipo == SIMB_VAR_LOCAL
    int num_params;                // Usado quando tipo == SIMB_FUNCAO
    int num_locais;                // Usado quando tipo == SIMB_FUNCAO
    TabelaSimbolos *tabela_local;  // Usado quando tipo == SIMB_FUNCAO
    struct NoHash *prox;
} NoHash;

struct TabelaSimbolos {
    NoHash *entradas[TAMANHO_TABELA];
};

// Criação e liberação (libera recursivamente tabelas locais se houver)
TabelaSimbolos *criaTabelaSimbolos(void);
void liberaTabelaSimbolos(TabelaSimbolos *tabela);

// Inserções por categoria
void inserirVarGlobal(TabelaSimbolos *tabela, const char *nome);
void inserirVarLocal(TabelaSimbolos *tabela, const char *nome, int offset);
void inserirFuncao(TabelaSimbolos *tabela, const char *nome,
                   int num_params, int num_locais,
                   TabelaSimbolos *tabela_local);

// Busca retornando o nó completo (ou NULL se não existir)
NoHash *buscarSimbolo(TabelaSimbolos *tabela, const char *nome);

#endif