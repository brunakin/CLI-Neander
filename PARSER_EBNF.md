## Parser (Etapa 1) — Gramática (EBNF)

Linguagem mínima para expressões matemáticas com precedência e parênteses.

```ebnf
expressao = termo { ("+" | "-") termo } ;
termo     = fator { ("*" | "/") fator } ;
fator     = INT | "(" expressao ")" | ("+" | "-") fator ;

INT    = digit { digit } ;
digit  = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;
```

### Observações

- Aceita espaços em branco em qualquer lugar.
- Inteiros são interpretados como base 10.
- Erros indicam a **posição** (índice) na string de entrada.

