#ifndef GERADOR_H
#define GERADOR_H

#include <stdio.h>
#include "../analise_sintatica/ast.h"
#include "../analise_semantica/tabela_simbolos.h"

int gerar_codigo_ast(Program *programa, TabelaSimbolos *tabelaGlobal, FILE *saida);

#endif