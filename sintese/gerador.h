#ifndef GERADOR_H
#define GERADOR_H

#include <stdio.h>
#include "../analise_sintatica/ast.h"

/* Emite um programa x86-64 completo; o runtime.s deve estar acessível ao assembler. */
int gerar_codigo_ast(Program *programa, FILE *saida);

#endif
