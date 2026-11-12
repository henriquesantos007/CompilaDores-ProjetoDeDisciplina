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

// Função para liberar a memória da tabela inteira no final
void liberaTabelaSimbolos(TabelaSimbolos *tabela) {
    for (int i = 0; i < TAMANHO_TABELA; i++) {
        NoHash *atual = tabela->entradas[i];
        while (atual != NULL) {
            NoHash *prox = atual->prox;
            free(atual->nome_variavel);
            free(atual);
            atual = prox;
        }
    }
    free(tabela);
}

// Insere uma nova variável na tabela (no início da lista encadeada)
void inserirTabela(TabelaSimbolos *tabela, const char *nome) {
    unsigned int indice = calcularHash(nome);
    
    // Aloca o novo nó
    NoHash *novo_no = malloc(sizeof(NoHash));
    if (novo_no == NULL) {
        // Tratamento padrão de erro de memória
        exit(1); 
    }
    
    // Aloca e copia o nome da string
    novo_no->nome_variavel = malloc(strlen(nome) + 1);
    strcpy(novo_no->nome_variavel, nome);
    
    // O novo nó aponta para quem era o antigo primeiro da lista
    novo_no->prox = tabela->entradas[indice];
    
    // A gaveta agora aponta para o novo nó como o primeiro da lista
    tabela->entradas[indice] = novo_no;
}

// Busca se uma variável já existe na tabela
bool buscarTabela(TabelaSimbolos *tabela, const char *nome) {
    unsigned int indice = calcularHash(nome);
    
    // Pega o primeiro nó da gaveta
    NoHash *atual = tabela->entradas[indice];
    
    // Percorre a lista encadeada procurando a string
    while (atual != NULL) {
        if (strcmp(atual->nome_variavel, nome) == 0) {
            return true; // Encontrou
        }
        atual = atual->prox;
    }
    
    return false; // Chegou no fim da lista e não achou
}