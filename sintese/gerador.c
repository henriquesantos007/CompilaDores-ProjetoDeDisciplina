#include <stdio.h>
#include "ast.h"
#include "gerador.h"

// Função auxiliar recursiva para as contas matemáticas
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
        fprintf(saida, "    popq %%rbx\n"); // Lado direito
        fprintf(saida, "    popq %%rax\n"); // Lado esquerdo

        // Resolve a matemática baseada no operador
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
                // O professor falou em sala que a divisão no x86-64 exige zerar o registrador %rdx antes de dividir
                fprintf(saida, "    cqto\n");
                fprintf(saida, "    idivq %%rbx\n");
                break;
        }

        // Devolve o resultado final para a pilha
        fprintf(saida, "    pushq %%rax\n");
    }
}

// Motor principal de Geração de Código
int gerar_codigo_ast(Program *programa, FILE *saida) {
    if (programa == NULL || saida == NULL) return 1;

    // Geração da seção .bss (é a memória das variáveis)
    fprintf(saida, ".section .bss\n");
    Declaration *decl = programa->declaracoes;
    while (decl != NULL) {
        // Reserva 8 bytes para cada variável (arquitetura x86-64)
        fprintf(saida, "    .lcomm %s, 8\n", decl->identificador);
        decl = decl->prox;
    }
    fprintf(saida, "\n");

    // Cabeçalho da seção .text (é o Código)
    fprintf(saida, ".section .text\n");
    fprintf(saida, ".globl _start\n");
    fprintf(saida, "_start:\n");

    // Cálculo e atribuições das declarações
    decl = programa->declaracoes; // Rebobina o ponteiro para o início da lista
    while (decl != NULL) {
        // Calcula a expressão (o resultado final da conta vai sobrar no topo da pilha)
        gerar_codigo_expressao(decl->expressao, saida);
        
        // Desempilha o resultado para %rax e salva na memória da variável
        fprintf(saida, "    popq %%rax\n");
        fprintf(saida, "    movq %%rax, %s\n", decl->identificador);
        
        decl = decl->prox;
    }

    // Cálculo da expressão final
    gerar_codigo_expressao(programa->expressaoFinal, saida);

    // O resultado da expressão final fica no topo da pilha.
    // Desempilhamos para %rax, que é o registrador que a função imprime_num do runtime.s espera ler
    fprintf(saida, "    popq %%rax\n");

    // 5. FINALIZAÇÃO PADRÃO
	fprintf(saida, "    call imprime_num\n");
    fprintf(saida, "    call sair\n");
    fprintf(saida, ".include \"runtime.s\"\n");

	return 0;
}