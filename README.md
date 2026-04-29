# CLI Neander

## ADO 1 - Compiladores

## Etapa 1 — Parser de expressões

### Compilar e rodar (Windows / gcc)

```bash
gcc -std=c99 -Wall -Wextra -o parser_cli parser_cli.c parser.c
.\parser_cli "2+3*(4-1)"
```

Ou sem argumento (ele pede no stdin):

```bash
.\parser_cli
```

## Etapa 2 — Assembler (duas passagens)

Arquivos:

- `assembler.h` / `assembler.c`: montagem em duas passagens + saída `.mem`
- `assembler_cli.c`: CLI simples do assembler
- `exemplo.asm`: programinha de exemplo

### Compilar e montar um programa

```bash
gcc -std=c99 -Wall -Wextra -o montar assembler_cli.c assembler.c
.\montar exemplo.asm saida.mem
```

## Executor (já pronto)

### A especificação do tipo de valores (Decimal ou Hexadecimal) é colocada no comando da execução

#### Comandos:

gcc -o neander main.c

./neander saida.mem -hex

./neander saida.mem -dec
