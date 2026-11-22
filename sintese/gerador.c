#include <stdio.h>
#include "ast.h"
#include "gerador.h"

// Contador global estático para garantir nomes únicos de rótulos (Lfalso0, Lfim0, etc.)
static int contador_rotulos = 0;

// Função auxiliar recursiva para as contas matemáticas e comparações
static void gerar_codigo_expressao(Exp *exp, FILE *saida) {
    if (exp == NULL) return;

    if (exp->tipo == NO_CONST) {
        // Empilha o número diretamente
        fprintf(saida, "    pushq $%d\n", exp->dados.valor);
    } 
    else if (exp->tipo == NO_IDENT) {
        // Traz o valor da memória para %rax, e depois empilha
        fprintf(saida, "    movq %s, %%rax\n", exp->dados.identificador);
        fprintf(saida, "    pushq %%rax\n");
    } 
    else if (exp->tipo == NO_OPBIN) {
        // Gera o código das subexpressões
        gerar_codigo_expressao(exp->dados.opbin.esq, saida);
        gerar_codigo_expressao(exp->dados.opbin.dir, saida);

        // Lógica de desempilhar
        fprintf(saida, "    popq %%rbx\n"); // Lado direito (B)
        fprintf(saida, "    popq %%rax\n"); // Lado esquerdo (A)

        // Resolve a operação baseada no operador
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
                // Extensão de sinal de %rax para %rdx:%rax antes da divisão
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

        // Devolve o resultado final (número ou 0/1 booleano) para a pilha
        fprintf(saida, "    pushq %%rax\n");
    }
}

// Função auxiliar recursiva para gerar código dos comandos (atribuição, if/else, while)
static void gerar_codigo_comandos(Cmd *cmd, FILE *saida) {
    while (cmd != NULL) {
        if (cmd->tipo == CMD_ATRIB) {
            gerar_codigo_expressao(cmd->dados.cmdAtrib.expressao, saida);
            fprintf(saida, "    popq %%rax\n");
            fprintf(saida, "    movq %%rax, %s\n", cmd->dados.cmdAtrib.identificador);
        } 
        else if (cmd->tipo == CMD_IF) {
            int id = contador_rotulos++;

            // Avalia a condição e desempilha o resultado para %rax
            gerar_codigo_expressao(cmd->dados.cmdIf.condicao, saida);
            fprintf(saida, "    popq %%rax\n");
            fprintf(saida, "    cmpq $0, %%rax\n");
            fprintf(saida, "    jz Lfalso%d\n", id);

            // Bloco verdadeiro (if)
            gerar_codigo_comandos(cmd->dados.cmdIf.blocoIf, saida);
            fprintf(saida, "    jmp Lfim%d\n", id);

            // Bloco falso (else)
            fprintf(saida, "Lfalso%d:\n", id);
            gerar_codigo_comandos(cmd->dados.cmdIf.blocoElse, saida);

            fprintf(saida, "Lfim%d:\n", id);
        } 
        else if (cmd->tipo == CMD_WHILE) {
            int id = contador_rotulos++;

            fprintf(saida, "Linicio%d:\n", id);

            // Avalia a condição e desempilha o resultado para %rax
            gerar_codigo_expressao(cmd->dados.cmdWhile.condicao, saida);
            fprintf(saida, "    popq %%rax\n");
            fprintf(saida, "    cmpq $0, %%rax\n");
            fprintf(saida, "    jz Lfim%d\n", id);

            // Corpo do laço
            gerar_codigo_comandos(cmd->dados.cmdWhile.corpo, saida);
            fprintf(saida, "    jmp Linicio%d\n", id);

            fprintf(saida, "Lfim%d:\n", id);
        }

        cmd = cmd->prox;
    }
}

// Motor principal de Geração de Código
int gerar_codigo_ast(Program *programa, FILE *saida) {
    if (programa == NULL || saida == NULL) return 1;

    // Reseta o contador de rótulos a cada compilação
    contador_rotulos = 0;

    // Geração da seção .bss (memória das variáveis)
    fprintf(saida, ".section .bss\n");
    Declaration *decl = programa->declaracoes;
    while (decl != NULL) {
        // Reserva 8 bytes para cada variável (arquitetura x86-64)
        fprintf(saida, "    .lcomm %s, 8\n", decl->identificador);
        decl = decl->prox;
    }
    fprintf(saida, "\n");

    // Cabeçalho da seção .text (código executável)
    fprintf(saida, ".section .text\n");
    fprintf(saida, ".globl _start\n");
    fprintf(saida, "_start:\n");

    // Cálculo e atribuições das declarações iniciais
    decl = programa->declaracoes;
    while (decl != NULL) {
        gerar_codigo_expressao(decl->expressao, saida);
        fprintf(saida, "    popq %%rax\n");
        fprintf(saida, "    movq %%rax, %s\n", decl->identificador);
        decl = decl->prox;
    }

    // Geração de código dos comandos do corpo { ... }
    gerar_codigo_comandos(programa->comandos, saida);

    // Cálculo da expressão final (return)
    gerar_codigo_expressao(programa->expressaoFinal, saida);
    fprintf(saida, "    popq %%rax\n");

    // Finalização padrão (imprime %rax e encerra o processo)
    fprintf(saida, "    call imprime_num\n");
    fprintf(saida, "    call sair\n");
    fprintf(saida, ".include \"runtime.s\"\n");

    return 0;
}