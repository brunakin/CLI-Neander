#ifndef NEANDER_PARSER_H
#define NEANDER_PARSER_H

#include <stddef.h>

// Parser simples de expressões matemáticas.

// Gramática (EBNF):
//   expressao = termo { ("+" | "-") termo } ;
//   termo     = fator { ("*" | "/") fator } ;
//   fator     = NUM | "(" expressao ")" | ("+" | "-") fator ;


int analisar_expressao(const char* entrada,
                       long* resultado,
                       char* msg_erro,
                       size_t tam_msg_erro);

#endif

