#!/bin/sh
set -eu

raiz=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
temporario=$(mktemp -d)
trap 'rm -rf "$temporario"' EXIT HUP INT TERM

cd "$raiz"

gcc -std=c11 -Wall -Wextra -Werror \
    -Ianalise_lexica -Ianalise_sintatica -Isintese \
    main.c analise_lexica/lexer.c analise_sintatica/parser.c \
    analise_sintatica/ast.c sintese/gerador.c \
    -o "$temporario/compilador"

    testar_programa() {
    entrada=$1
    esperado=$2

    printf '%s\n' "$entrada" > "$temporario/entrada.ev"

    "$temporario/compilador" "$temporario/entrada.ev" \
        > "$temporario/saida.txt"

    if ! grep -Fq "$esperado" "$temporario/saida.txt"; then
        printf 'Falha: resultado inesperado para:\n%s\n' "$entrada" >&2
        cat "$temporario/saida.txt" >&2
        exit 1
    fi

    printf 'OK: programa aceito\n'
}

testar_programa 'x = 10;
y = 20;
= x + y * 2' '= (x + (y * 2))'

testar_programa '= 10 - 3 - 2' '= ((10 - 3) - 2)'

testar_programa '= 10 * (3 + 2)' '= (10 * (3 + 2))'

testar_erro() {
    entrada=$1
    esperado=$2

    printf '%s\n' "$entrada" > "$temporario/entrada.ev"

    if "$temporario/compilador" "$temporario/entrada.ev" \
        > "$temporario/saida.txt" 2>&1; then
        printf 'Falha: programa inválido foi aceito:\n%s\n' "$entrada" >&2
        exit 1
    fi

    if ! grep -Fq "$esperado" "$temporario/saida.txt"; then
        printf 'Falha: mensagem de erro inesperada:\n' >&2
        cat "$temporario/saida.txt" >&2
        exit 1
    fi

    printf 'OK: programa inválido rejeitado\n'
}

testar_erro 'x = 10
= x + 5' '; esperado'

testar_erro 'x = 10;
x + 5' '= esperado'