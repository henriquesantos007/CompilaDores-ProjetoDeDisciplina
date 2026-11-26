#include <stdio.h>
#include "ast.h"
#include "tabela_simbolos.h"
#include "gerador.h"

// Contador global estático para garantir nomes únicos de rótulos (Lfalso0, Lfim0, etc.)
static int contador_rotulos = 0;

static void gerar_codigo_expressao(Exp *exp, TabelaSimbolos *tabelaLocal, FILE *saida);

// Empilha os argumentos reais em ordem inversa (do último para o primeiro)
static void gerar_codigo_args_reverso(Arg *arg, TabelaSimbolos *tabelaLocal, FILE *saida) {
    if (arg == NULL) return;

    gerar_codigo_args_reverso(arg->prox, tabelaLocal, saida);
    // gerar_codigo_expressao já deixa o resultado do argumento no topo da pilha
    gerar_codigo_expressao(arg->expressao, tabelaLocal, saida);
}

// Função auxiliar recursiva para expressões matemáticas, comparações e chamadas
static void gerar_codigo_expressao(Exp *exp, TabelaSimbolos *tabelaLocal, FILE *saida) {
    if (exp == NULL) return;

    if (exp->tipo == NO_CONST) {
        fprintf(saida, "    pushq $%d\n", exp->dados.valor);
    } 
    else if (exp->tipo == NO_IDENT) {
        NoHash *simboloLocal = buscarSimbolo(tabelaLocal, exp->dados.identificador);
        if (simboloLocal != NULL) {
            // Variável local ou parâmetro: acesso indireto relativo a %rbp
            fprintf(saida, "    movq %d(%%rbp), %%rax\n", simboloLocal->offset);
        } else {
            // Variável global: acesso direto na seção .bss
            fprintf(saida, "    movq %s, %%rax\n", exp->dados.identificador);
        }
        fprintf(saida, "    pushq %%rax\n");
    } 
    else if (exp->tipo == NO_CHAMADA) {
        // Empilha os argumentos reais na ordem inversa
        gerar_codigo_args_reverso(exp->dados.chamada.argumentos, tabelaLocal, saida);

        // Chama a função (empilha endereço de retorno e salta para o rótulo)
        fprintf(saida, "    call %s\n", exp->dados.chamada.nome);

        // Remove os argumentos da pilha após o retorno
        if (exp->dados.chamada.numArgs > 0) {
            fprintf(saida, "    addq $%d, %%rsp\n", exp->dados.chamada.numArgs * 8);
        }

        // O valor de retorno vem em %rax; empilha para manter a convenção de expressões
        fprintf(saida, "    pushq %%rax\n");
    }
    else if (exp->tipo == NO_OPBIN) {
        gerar_codigo_expressao(exp->dados.opbin.esq, tabelaLocal, saida);
        gerar_codigo_expressao(exp->dados.opbin.dir, tabelaLocal, saida);

        fprintf(saida, "    popq %%rbx\n"); // Lado direito (B)
        fprintf(saida, "    popq %%rax\n"); // Lado esquerdo (A)

        switch (exp->dados.opbin.op) {
            case OP_SOMA:
                fprintf(saida, "    addq %%rbx, %%rax\n");
                break;
            case OP_SUB:
                fprintf(saida, "    subq %%rbx, %%rax\n");
                break;
            case OP_MULT:
                fprintf(saida, "    imulq %%rbx, %%rax\n");
                break;
            case OP_DIV:
                fprintf(saida, "    cqto\n");
                fprintf(saida, "    idivq %%rbx\n");
                break;
            case OP_IGUAL:
                fprintf(saida, "    xorq %%rcx, %%rcx\n");
                fprintf(saida, "    cmpq %%rbx, %%rax\n");
                fprintf(saida, "    setz %%cl\n");
                fprintf(saida, "    movq %%rcx, %%rax\n");
                break;
            case OP_MENOR:
                fprintf(saida, "    xorq %%rcx, %%rcx\n");
                fprintf(saida, "    cmpq %%rbx, %%rax\n");
                fprintf(saida, "    setl %%cl\n");
                fprintf(saida, "    movq %%rcx, %%rax\n");
                break;
            case OP_MAIOR:
                fprintf(saida, "    xorq %%rcx, %%rcx\n");
                fprintf(saida, "    cmpq %%rbx, %%rax\n");
                fprintf(saida, "    setg %%cl\n");
                fprintf(saida, "    movq %%rcx, %%rax\n");
                break;
        }

        fprintf(saida, "    pushq %%rax\n");
    }
}

// Função auxiliar recursiva para gerar código dos comandos (atribuição, if/else, while)
static void gerar_codigo_comandos(Cmd *cmd, TabelaSimbolos *tabelaLocal, FILE *saida) {
    while (cmd != NULL) {
        if (cmd->tipo == CMD_ATRIB) {
            gerar_codigo_expressao(cmd->dados.cmdAtrib.expressao, tabelaLocal, saida);
            fprintf(saida, "    popq %%rax\n");

            NoHash *simboloLocal = buscarSimbolo(tabelaLocal, cmd->dados.cmdAtrib.identificador);
            if (simboloLocal != NULL) {
                fprintf(saida, "    movq %%rax, %d(%%rbp)\n", simboloLocal->offset);
            } else {
                fprintf(saida, "    movq %%rax, %s\n", cmd->dados.cmdAtrib.identificador);
            }
        } 
        else if (cmd->tipo == CMD_IF) {
            int id = contador_rotulos++;

            gerar_codigo_expressao(cmd->dados.cmdIf.condicao, tabelaLocal, saida);
            fprintf(saida, "    popq %%rax\n");
            fprintf(saida, "    cmpq $0, %%rax\n");
            fprintf(saida, "    jz Lfalso%d\n", id);

            gerar_codigo_comandos(cmd->dados.cmdIf.blocoIf, tabelaLocal, saida);
            fprintf(saida, "    jmp Lfim%d\n", id);

            fprintf(saida, "Lfalso%d:\n", id);
            gerar_codigo_comandos(cmd->dados.cmdIf.blocoElse, tabelaLocal, saida);

            fprintf(saida, "Lfim%d:\n", id);
        } 
        else if (cmd->tipo == CMD_WHILE) {
            int id = contador_rotulos++;

            fprintf(saida, "Linicio%d:\n", id);

            gerar_codigo_expressao(cmd->dados.cmdWhile.condicao, tabelaLocal, saida);
            fprintf(saida, "    popq %%rax\n");
            fprintf(saida, "    cmpq $0, %%rax\n");
            fprintf(saida, "    jz Lfim%d\n", id);

            gerar_codigo_comandos(cmd->dados.cmdWhile.corpo, tabelaLocal, saida);
            fprintf(saida, "    jmp Linicio%d\n", id);

            fprintf(saida, "Lfim%d:\n", id);
        }

        cmd = cmd->prox;
    }
}

// Gera prólogo, inicialização de locais, corpo, retorno e epílogo de uma função
static void gerar_codigo_funcao(FunDecl *fun, TabelaSimbolos *tabelaGlobal, FILE *saida) {
    NoHash *simbFunc = buscarSimbolo(tabelaGlobal, fun->nome);
    TabelaSimbolos *tabelaLocal = (simbFunc != NULL) ? simbFunc->tabela_local : NULL;
    int bytesLocais = fun->numLocais * 8;

    // Rótulo e prólogo da função
    fprintf(saida, "\n%s:\n", fun->nome);
    fprintf(saida, "    pushq %%rbp\n");
    if (bytesLocais > 0) {
        fprintf(saida, "    subq $%d, %%rsp\n", bytesLocais);
    }
    fprintf(saida, "    movq %%rsp, %%rbp\n");

    // Inicialização das variáveis locais (offset = i * 8)
    int i = 0;
    VarDecl *var = fun->locais;
    while (var != NULL) {
        gerar_codigo_expressao(var->expressao, tabelaLocal, saida);
        fprintf(saida, "    popq %%rax\n");
        fprintf(saida, "    movq %%rax, %d(%%rbp)\n", i * 8);
        i++;
        var = var->prox;
    }

    // Comandos do corpo da função
    gerar_codigo_comandos(fun->comandos, tabelaLocal, saida);

    // Expressão de retorno (resultado final fica em %rax)
    gerar_codigo_expressao(fun->expressaoFinal, tabelaLocal, saida);
    fprintf(saida, "    popq %%rax\n");

    // Epílogo da função
    if (bytesLocais > 0) {
        fprintf(saida, "    addq $%d, %%rsp\n", bytesLocais);
    }
    fprintf(saida, "    popq %%rbp\n");
    fprintf(saida, "    ret\n");
}

// Motor principal de Geração de Código
int gerar_codigo_ast(Program *programa, TabelaSimbolos *tabelaGlobal, FILE *saida) {
    if (programa == NULL || tabelaGlobal == NULL || saida == NULL) return 1;

    contador_rotulos = 0;

    // Seção .bss (aloca 8 bytes para cada variável global)
    fprintf(saida, ".section .bss\n");
    Decl *decl = programa->declaracoes;
    while (decl != NULL) {
        if (decl->tipo == DECL_VAR) {
            fprintf(saida, "    .lcomm %s, 8\n", decl->dados.varDecl->identificador);
        }
        decl = decl->prox;
    }
    fprintf(saida, "\n");

    // Seção .text e ponto de entrada _start (bloco main)
    fprintf(saida, ".section .text\n");
    fprintf(saida, ".globl _start\n");
    fprintf(saida, "_start:\n");

    // Inicialização das variáveis globais (escopo local = NULL)
    decl = programa->declaracoes;
    while (decl != NULL) {
        if (decl->tipo == DECL_VAR) {
            VarDecl *var = decl->dados.varDecl;
            gerar_codigo_expressao(var->expressao, NULL, saida);
            fprintf(saida, "    popq %%rax\n");
            fprintf(saida, "    movq %%rax, %s\n", var->identificador);
        }
        decl = decl->prox;
    }

    // Comandos do bloco main
    gerar_codigo_comandos(programa->comandos, NULL, saida);

    // Expressão final (return) do bloco main
    gerar_codigo_expressao(programa->expressaoFinal, NULL, saida);
    fprintf(saida, "    popq %%rax\n");

    // Finalização do programa principal
    fprintf(saida, "    call imprime_num\n");
    fprintf(saida, "    call sair\n");

    // Emissão do código das funções declaradas
    decl = programa->declaracoes;
    while (decl != NULL) {
        if (decl->tipo == DECL_FUN) {
            gerar_codigo_funcao(decl->dados.funDecl, tabelaGlobal, saida);
        }
        decl = decl->prox;
    }

    fprintf(saida, "\n.include \"runtime.s\"\n");

    return 0;
}