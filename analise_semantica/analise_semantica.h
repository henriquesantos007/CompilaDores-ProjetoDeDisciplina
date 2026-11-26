#ifndef ANALISE_SEMANTICA_H
#define ANALISE_SEMANTICA_H

#include "../analise_sintatica/ast.h"
#include "tabela_simbolos.h"

TabelaSimbolos *analisarSemantica(Program *programa);

#endif