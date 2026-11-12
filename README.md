# Atividade 07 — Compilador EV (Análise Semântica e Geração de Código)

**Disciplina:** Construção de Compiladores  
**Plataforma alvo:** x86-64 / Linux (GNU Assembler)

## 1. Grupo

| Nome                               | Matrícula   |
| ---------------------------------- | ----------- |
| Allan Gabriel da Cunha Vasconcelos | 20240009142 |
| Luiz Henrique Santos da Graça      | 20240008261 |
| Maria Eloiza Monteiro de Souza     | 20240008280 |
| Vinicius Victor Lopes de Almeida   | 20240008181 |

## 2. Etapa Léxica

O analisador léxico lê o código-fonte de um arquivo e o converte em uma sequência de tokens. Na linguagem EV, espaços, tabulações e quebras de linha são ignorados. Os seguintes tokens são reconhecidos:

| Token           | Lexemas reconhecidos |
| --------------- | -------------------- |
| `IDENTIFICADOR` | Nomes de variáveis (ex: `x`, `idade123`, `ABC`) |
| `NUM`           | Números inteiros (ex: `321`) |
| `OP`            | Operadores matemáticos (`+`, `-`, `*`, `/`) |
| `IGUAL`         | Atribuição e marcador de expressão final (`=`) |
| `PONTO_VIRGULA` | Fim de declaração de variável (`;`) |
| `ABRE_PAR`      | `(` |
| `FECHA_PAR`     | `)` |
| `TOKEN_EOF`     | Fim do arquivo |
| `ERRO`          | Caractere não reconhecido |

## 3. Etapa Sintática

O analisador sintático (`parser.h`/`parser.c`) constrói a Árvore Sintática Abstrata (AST) validando a gramática da linguagem EV através de análise descendente recursiva. A nova gramática exige a declaração de variáveis antes da expressão final, sendo a expressão final precedida obrigatoriamente por um `=` como sinalizador:

```text
<programa>  ::= <decl>* '=' <expressao>
<decl>      ::= <identificador> '=' <expressao> ';'
<expressao> ::= <exp_m> (('+' | '-') <exp_m>)*
<exp_m>     ::= <prim> (('*' | '/') <prim>)*
<prim>      ::= <num> | <identificador> | '(' <expressao> ')'
<ident>     ::= <letra><letra_digito>*
<num>       ::= <digito><digito>*
```

## 4. Etapa Semântica (Tabela de Símbolos)

Atuando como uma camada de proteção antes da geração de código, a Análise Semântica garante a validade do escopo:
- **Hash Table:** Implementada com algoritmo *djb2* e tratamento de colisões por encadeamento separado garantindo alta performance (`O(1)`) nas buscas e inserções.
- **Validação de Variáveis:** O compilador percorre a AST registrando as variáveis declaradas. Caso a expressão matemática tente utilizar uma variável "fantasma" (não declarada previamente), o motor semântico emite um erro formatado em `stderr` e interrompe imediatamente o fluxo com `exit(1)`.

## 5. Etapa de Síntese (Geração de Código x86-64)

O motor de síntese (`gerador.c`) traduz a AST validada em um arquivo Assembly executável organizado em duas seções de hardware:
- **Seção `.bss`:** Percorre a lista de declarações e utiliza a diretiva `.lcomm` para alocar dinamicamente 8 bytes de memória RAM (arquitetura 64 bits) para cada variável identificada.
- **Seção `.text`:** Processa o cálculo matemático utilizando uma arquitetura LIFO baseada em pilha. Constantes e variáveis são lidas da memória e empilhadas (`pushq`). Operadores binários desempilham os operandos em registradores voláteis (`popq`), executam as instruções da ULA (`addq`, `subq`, `imulq`, `idivq`), e empilham o resultado de volta. Ao fim do laço, o valor é armazenado na RAM via `movq`.

## 6. Como Executar e Testar

### Pré-requisitos
Ambiente Linux com as seguintes ferramentas de compilação C e baixo nível instaladas:
- `gcc` (Compilador C)
- `as` (GNU Assembler)
- `ld` (GNU Linker)

### 🤖 1. Execução Automatizada (Bateria de Testes)
O projeto contém um script robusto de CI (Integração Contínua) que compila o motor C, gera os binários em Assembly e testa a linguagem contra diversos cenários matemáticos para prevenir regressões de memória ou sintaxe.

Na raiz do projeto, dê permissão e rode o script:
```bash
chmod +x tests/test_sintese.sh
./tests/test_sintese.sh
```

### 🛠️ 2. Execução Manual Passo a Passo
Para compilar visualmente o ciclo de vida completo de um código fonte (`.txt` / `.ev`), siga o pipeline de construção abaixo:

**Passo A: Compilar o Motor do Compilador (C)**
```bash
gcc -std=c11 -Wall -Wextra -Werror \
    -Ianalise_lexica -Ianalise_sintatica -Ianalise_semantica -Isintese \
    main.c \
    analise_lexica/lexer.c \
    analise_sintatica/parser.c analise_sintatica/ast.c \
    analise_semantica/analise_semantica.c analise_semantica/tabela_simbolos.c \
    sintese/gerador.c \
    -o compilador
```

**Passo B: Traduzir o código fonte (EV) para Assembly (x86-64)**
Acrescente a flag `--asm`. O compilador omitirá a árvore e imprimirá estritamente o código x86-64. Redirecione a saída para salvar o arquivo. 
```bash
./compilador --asm tests/exemplo03.txt > programa.s
```

Caso queira visualizar somente a Sequência de Tokens e a Árvore Sintática, remova o `--asm` e NÃO redirecione a saída:

```bash
./compilador tests/exemplo03.txt 
```

**Passo C: Montar e Linkar o Executável Final**
*(Execute estritamente na raiz do projeto para injetar com sucesso o `runtime.s`)*
```bash
as --64 programa.s -o programa.o
ld programa.o -o programa
```

**Passo D: Rodar o Software Gerado**
```bash
./programa
```

## 7. Arquivos de Exemplo e Testes

A pasta `tests/` possui arquivos manuais estritamente adequados à nova gramática da Atividade 07:
- `tests/exemplo01.txt`: Atribuição declarativa e retorno simples.
- `tests/exemplo02.txt`: Múltiplas variáveis orquestradas em uma operação binária.
- `tests/exemplo03.txt`: Cálculos compostos avançados (área geométrica) aplicando regras de escopo e precedência matemática completa.
- `tests/test_sintese.sh`: Script de validação automática *end-to-end*.

## 8. Organização da Arquitetura

O projeto adota o Princípio da Responsabilidade Única (SRP) e separação estrutural de pastas:
- `analise_lexica/`: Implementação do Lexer e mapeamento cru de Tokens.
- `analise_sintatica/`: Implementação do Parser Descendente Recursivo e alocação da Árvore Sintática Abstrata (AST).
- `analise_semantica/`: Validador de contexto protegido e motor nativo da Tabela de Símbolos (Hash Table).
- `sintese/`: Motor gerador de código Assembly x86-64 e orquestrador de registradores e seções de memória.
- `runtime.s`: Funções de sistema customizadas (`imprime_num` e `sair`), ligadas intrinsecamente no assembly final.
- `main.c`: Orquestrador de alto nível do pipeline, responsável unicamente pelo tráfego e injeção de dependências.

## 9. Uso de LLMs

As etapas de análise léxica e sintática contaram com apoio inicial (conforme entregas anteriores). Para a **Atividade 07**, a engenharia da Análise Semântica via Tabela Hash (recomendação e implementação do algoritmo *djb2* O(1)), debugging, geração do `README.md` e arquivos de testes além do *refactoring* do `main.c` focado em princípios de *Clean Code*, foram estruturados e depurados colaborativamente sob a mentoria avançada da Inteligência Artificial do Google Gemini.