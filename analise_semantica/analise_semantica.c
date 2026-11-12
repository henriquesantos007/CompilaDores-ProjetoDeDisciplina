#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "tabela_simbolos.h"

// Função principal da análise semântica
void analisarSemantica(Program *programa) {
    // Cria a tabela de verificação (Hash Table)
    TabelaSimbolos *tabela = criaTabelaSimbolos();
    
    // O laço de travessia das declarações do programa
    Declaration *declaracao = programa->declaracoes;
    while (declaracao != NULL) {
        
        // Verificar se as variáveis DENTRO da expressão existem
        verificaExpressao(tabela, declaracao->expressao);
        
        // Registrar a nova variável na tabela
        inserirTabela(tabela, declaracao->identificador);
        
        // Avança para a próxima declaração
        declaracao = declaracao->prox;
    }
    
    // Após ler todas as declarações, verifica a expressão final do programa
    verificaExpressao(tabela, programa->expressaoFinal);
    
    // Se chegou até aqui sem disparar erros, o programa está semanticamente correto!
    liberaTabelaSimbolos(tabela);
}

// Validar a expressão dependendo do tipo de Nó na expressão
static void verificaExpressao(TabelaSimbolos *tabela, Exp *exp) {
    if (exp == NULL) return;

    if (exp->tipo == NO_CONST) {
        // Não faz nada, números constantes são sempre seguros
        return;
    } 
    else if (exp->tipo == NO_OPBIN) {
        // Lógica recursiva para varrer os nós à esquerda e direita do nó de operação
        verificaExpressao(tabela, exp->dados.opbin.esq);
        verificaExpressao(tabela, exp->dados.opbin.dir);
    } 
    else if (exp->tipo == NO_IDENT) {
        // verifica a existência da variável na tabela de símbolos
        bool existe = buscarTabela(tabela, exp->dados.identificador);
        
        if (!existe) {
            fprintf(stderr, "Erro Semantico: A variavel '%s' nao foi declarada antes do uso.\n", exp->dados.identificador);
            exit(1);
        }
    }
}