#!/bin/sh
set -eu

raiz=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
temporario=$(mktemp -d)
trap 'rm -rf "$temporario"' EXIT HUP INT TERM

cd "$raiz"

gcc -std=c11 -Wall -Wextra -Werror \
    -Ianalise_lexica -Ianalise_sintatica -Ianalise_semantica -Isintese \
    main.c \
    analise_lexica/lexer.c \
    analise_sintatica/parser.c analise_sintatica/ast.c \
    analise_semantica/analise_semantica.c analise_semantica/tabela_simbolos.c \
    sintese/gerador.c \
    -o "$temporario/compilador"

testar_arquivo() {
    arquivo=$1
    esperado=$2

    "$temporario/compilador" --asm "$arquivo" > "$temporario/programa.s"

    as --64 "$temporario/programa.s" -o "$temporario/programa.o"
    ld "$temporario/programa.o" -o "$temporario/programa"

    resultado=$("$temporario/programa")
    if [ "$resultado" != "$esperado" ]; then
        printf 'Falha: %s gerou %s; esperado %s\n' "$arquivo" "$resultado" "$esperado" >&2
        exit 1
    fi
    printf 'OK: %s = %s\n' "$arquivo" "$resultado"
}

testar_erro() {
    arquivo=$1

    if "$temporario/compilador" --asm "$arquivo" > "$temporario/erro.s" 2> "$temporario/erro.txt"; then
        printf 'Falha: %s deveria ter sido rejeitado\n' "$arquivo" >&2
        exit 1
    fi
    printf 'OK: %s rejeitado (%s)\n' "$arquivo" "$(cat "$temporario/erro.txt")"
}

echo "Iniciando testes da linguagem Cmd (Atividade 08)..."

testar_arquivo tests/exemplo01.txt 42
testar_arquivo tests/exemplo02.txt 10
testar_arquivo tests/exemplo03.txt 100
testar_arquivo tests/exemplo04.txt 16
testar_arquivo tests/exemplo05.txt 55
testar_arquivo tests/exemplo06.txt 2
testar_arquivo tests/exemplo07.txt 6

echo "Testando entradas invalidas..."

testar_erro tests/erro01.txt
testar_erro tests/erro02.txt
testar_erro tests/erro03.txt

echo "Todos os testes passaram com sucesso!"
