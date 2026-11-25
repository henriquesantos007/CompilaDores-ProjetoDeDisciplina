#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "tabela_simbolos.h"
#include "analise_semantica.h"

// tabelaLocal é NULL quando estamos no escopo global (declarações globais e main)
static void verificaExpressao(TabelaSimbolos *global, TabelaSimbolos *local, Exp *exp);
static void verificaComandos(TabelaSimbolos *global, TabelaSimbolos *local, Cmd *cmd);
static void verificaFuncao(TabelaSimbolos *global, FunDecl *funDecl);

// Procura primeiro no escopo local e depois no global (shadowing)
static NoHash *buscaVariavel(TabelaSimbolos *global, TabelaSimbolos *local, const char *nome) {
    NoHash *simbolo = buscarSimbolo(local, nome);
    if (simbolo == NULL) {
        simbolo = buscarSimbolo(global, nome);
    }
    return simbolo;
}

// Função principal da análise semântica
TabelaSimbolos *analisarSemantica(Program *programa) {
    TabelaSimbolos *global = criaTabelaSimbolos();

    // Percorre as declarações na ordem em que aparecem no programa
    Decl *decl = programa->declaracoes;
    while (decl != NULL) {
        if (decl->tipo == DECL_VAR) {
            VarDecl *var = decl->dados.varDecl;
            verificaExpressao(global, NULL, var->expressao);
            inserirVarGlobal(global, var->identificador);
        }
        else if (decl->tipo == DECL_FUN) {
            verificaFuncao(global, decl->dados.funDecl);
        }
        decl = decl->prox;
    }

    // Bloco main só enxerga o escopo global
    verificaComandos(global, NULL, programa->comandos);
    verificaExpressao(global, NULL, programa->expressaoFinal);

    // A tabela não é liberada aqui porque a síntese ainda vai usar
    return global;
}

static void verificaFuncao(TabelaSimbolos *global, FunDecl *funDecl) {
    TabelaSimbolos *local = criaTabelaSimbolos();
    int L = funDecl->numLocais;

    // Registra a função antes do corpo para permitir recursão
    inserirFuncao(global, funDecl->nome, funDecl->numParams, L, local);

    // Parâmetros: offset = (L + 2 + j) * 8
    int j = 0;
    Param *param = funDecl->parametros;
    while (param != NULL) {
        inserirVarLocal(local, param->nome, (L + 2 + j) * 8);
        j++;
        param = param->prox;
    }

    // Variáveis locais: offset = i * 8
    int i = 0;
    VarDecl *var = funDecl->locais;
    while (var != NULL) {
        verificaExpressao(global, local, var->expressao);
        inserirVarLocal(local, var->identificador, i * 8);
        i++;
        var = var->prox;
    }

    verificaComandos(global, local, funDecl->comandos);
    verificaExpressao(global, local, funDecl->expressaoFinal);
}

static void verificaComandos(TabelaSimbolos *global, TabelaSimbolos *local, Cmd *cmd) {
    while (cmd != NULL) {
        if (cmd->tipo == CMD_ATRIB) {
            verificaExpressao(global, local, cmd->dados.cmdAtrib.expressao);

            NoHash *simbolo = buscaVariavel(global, local, cmd->dados.cmdAtrib.identificador);
            if (simbolo == NULL) {
                fprintf(stderr, "Erro Semantico: A variavel '%s' nao foi declarada e nao pode receber atribuicao.\n",
                        cmd->dados.cmdAtrib.identificador);
                exit(1);
            }
            if (simbolo->tipo == SIMB_FUNCAO) {
                fprintf(stderr, "Erro Semantico: '%s' e uma funcao e nao pode receber atribuicao.\n",
                        cmd->dados.cmdAtrib.identificador);
                exit(1);
            }
        }
        else if (cmd->tipo == CMD_IF) {
            verificaExpressao(global, local, cmd->dados.cmdIf.condicao);
            verificaComandos(global, local, cmd->dados.cmdIf.blocoIf);
            verificaComandos(global, local, cmd->dados.cmdIf.blocoElse);
        }
        else if (cmd->tipo == CMD_WHILE) {
            verificaExpressao(global, local, cmd->dados.cmdWhile.condicao);
            verificaComandos(global, local, cmd->dados.cmdWhile.corpo);
        }

        cmd = cmd->prox;
    }
}

// Validar a expressão dependendo do tipo de Nó na expressão
static void verificaExpressao(TabelaSimbolos *global, TabelaSimbolos *local, Exp *exp) {
    if (exp == NULL) return;

    if (exp->tipo == NO_CONST) {
        // Constantes são sempre válidas
        return;
    }
    else if (exp->tipo == NO_OPBIN) {
        verificaExpressao(global, local, exp->dados.opbin.esq);
        verificaExpressao(global, local, exp->dados.opbin.dir);
    }
    else if (exp->tipo == NO_IDENT) {
        NoHash *simbolo = buscaVariavel(global, local, exp->dados.identificador);

        if (simbolo == NULL) {
            fprintf(stderr, "Erro Semantico: A variavel '%s' nao foi declarada antes do uso.\n",
                    exp->dados.identificador);
            exit(1);
        }
        if (simbolo->tipo == SIMB_FUNCAO) {
            fprintf(stderr, "Erro Semantico: '%s' e uma funcao e nao pode ser usada como variavel.\n",
                    exp->dados.identificador);
            exit(1);
        }
    }
    else if (exp->tipo == NO_CHAMADA) {
        // Funções ficam sempre na tabela global
        NoHash *simbolo = buscarSimbolo(global, exp->dados.chamada.nome);

        if (simbolo == NULL) {
            fprintf(stderr, "Erro Semantico: A funcao '%s' nao foi declarada.\n",
                    exp->dados.chamada.nome);
            exit(1);
        }
        if (simbolo->tipo != SIMB_FUNCAO) {
            fprintf(stderr, "Erro Semantico: '%s' nao e uma funcao.\n",
                    exp->dados.chamada.nome);
            exit(1);
        }
        if (exp->dados.chamada.numArgs != simbolo->num_params) {
            fprintf(stderr, "Erro Semantico: A funcao '%s' espera %d argumento(s), mas recebeu %d.\n",
                    exp->dados.chamada.nome, simbolo->num_params, exp->dados.chamada.numArgs);
            exit(1);
        }

        Arg *arg = exp->dados.chamada.argumentos;
        while (arg != NULL) {
            verificaExpressao(global, local, arg->expressao);
            arg = arg->prox;
        }
    }
}
