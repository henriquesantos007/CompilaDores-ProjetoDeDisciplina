#!/bin/sh
set -eu

raiz=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
temporario=$(mktemp -d)
trap 'rm -rf "$temporario"' EXIT HUP INT TERM

cd "$raiz"

# Compilação Atualizada incluindo o motor semântico
gcc -std=c11 -Wall -Wextra -Werror \
    -Ianalise_lexica -Ianalise_sintatica -Ianalise_semantica -Isintese \
    main.c \
    analise_lexica/lexer.c \
    analise_sintatica/parser.c analise_sintatica/ast.c \
    analise_semantica/analise_semantica.c analise_semantica/tabela_simbolos.c \
    sintese/gerador.c \
    -o "$temporario/compilador"

testar_expressao() {
    codigo_ev=$1
    esperado=$2
    
    printf '%s\n' "$codigo_ev" > "$temporario/entrada.txt"
    
    # Roda o compilador
    "$temporario/compilador" --asm "$temporario/entrada.txt" > "$temporario/programa.s"
    
    # Monta e Linka o Assembly x86-64
    as --64 "$temporario/programa.s" -o "$temporario/programa.o"
    ld "$temporario/programa.o" -o "$temporario/programa"
    
    # Executa e Valida
    resultado=$("$temporario/programa")
    if [ "$resultado" != "$esperado" ]; then
        printf 'Falha: O codigo gerou %s; esperado %s\n' "$resultado" "$esperado" >&2
        exit 1
    fi
    printf 'OK: Resultado = %s\n' "$resultado"
}

echo "Iniciando testes de Sintese e Memoria (Atividade 07)..."

# Teste 1: Variavel Simples
testar_expressao 'x = 10; = x' '10'

# Teste 2: Multiplas Variaveis e Matematica
testar_expressao 'x = 5; y = 15; = (y - x)' '10'

# Teste 3: Variaveis Compostas e Precedencia
testar_expressao 'base = 10; altura = 20; area = (base * altura); = (area / 2)' '100'

# Teste 4: Regra de Atribuicao Cega (o compilador so calcula e armazena)
testar_expressao 'a = (11 - 7); b = 333; = a' '4'

echo "Todos os testes passaram com sucesso!"