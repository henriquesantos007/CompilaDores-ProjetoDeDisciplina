#include <stdio.h>
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
TabelaSimbolos *criaTabelaSimbolos(void) {
    TabelaSimbolos *tabela = malloc(sizeof(TabelaSimbolos));
    if (tabela == NULL) {
        fprintf(stderr, "Erro ao alocar memoria para a tabela de simbolos.\n");
        exit(1);
    }

    for (int i = 0; i < TAMANHO_TABELA; i++) {
        tabela->entradas[i] = NULL;
    }
    return tabela;
}

// Libera a tabela inteira (e as tabelas locais das funções)
void liberaTabelaSimbolos(TabelaSimbolos *tabela) {
    if (tabela == NULL) return;

    for (int i = 0; i < TAMANHO_TABELA; i++) {
        NoHash *atual = tabela->entradas[i];
        while (atual != NULL) {
            NoHash *prox = atual->prox;

            if (atual->tipo == SIMB_FUNCAO) {
                liberaTabelaSimbolos(atual->tabela_local);
            }

            free(atual->nome);
            free(atual);
            atual = prox;
        }
    }
    free(tabela);
}

// Cria um nó com valores padrão e coloca no início da lista da gaveta
static NoHash *inserirNo(TabelaSimbolos *tabela, const char *nome, TipoSimbolo tipo) {
    unsigned int indice = calcularHash(nome);

    NoHash *novo_no = malloc(sizeof(NoHash));
    if (novo_no == NULL) {
        fprintf(stderr, "Erro ao alocar memoria para a tabela de simbolos.\n");
        exit(1);
    }

    // Aloca e copia o nome da string
    novo_no->nome = malloc(strlen(nome) + 1);
    if (novo_no->nome == NULL) {
        fprintf(stderr, "Erro ao alocar memoria para a tabela de simbolos.\n");
        exit(1);
    }
    strcpy(novo_no->nome, nome);

    novo_no->tipo = tipo;
    novo_no->offset = 0;
    novo_no->num_params = 0;
    novo_no->num_locais = 0;
    novo_no->tabela_local = NULL;

    // O novo nó aponta para quem era o antigo primeiro da lista
    novo_no->prox = tabela->entradas[indice];
    tabela->entradas[indice] = novo_no;

    return novo_no;
}

void inserirVarGlobal(TabelaSimbolos *tabela, const char *nome) {
    inserirNo(tabela, nome, SIMB_VAR_GLOBAL);
}

void inserirVarLocal(TabelaSimbolos *tabela, const char *nome, int offset) {
    NoHash *no = inserirNo(tabela, nome, SIMB_VAR_LOCAL);
    no->offset = offset;
}

void inserirFuncao(TabelaSimbolos *tabela, const char *nome,
                   int num_params, int num_locais,
                   TabelaSimbolos *tabela_local) {
    NoHash *no = inserirNo(tabela, nome, SIMB_FUNCAO);
    no->num_params = num_params;
    no->num_locais = num_locais;
    no->tabela_local = tabela_local;
}

// Busca um símbolo pelo nome, retorna NULL se não existir
NoHash *buscarSimbolo(TabelaSimbolos *tabela, const char *nome) {
    if (tabela == NULL) return NULL;

    unsigned int indice = calcularHash(nome);
    NoHash *atual = tabela->entradas[indice];

    // Percorre a lista encadeada procurando a string
    while (atual != NULL) {
        if (strcmp(atual->nome, nome) == 0) {
            return atual;
        }
        atual = atual->prox;
    }

    return NULL;
}
