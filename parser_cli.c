#include "parser.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) {
    char buf[512];          // buffer pra ler do teclado 
    const char* src = NULL; // aponta pra expressão que vamos analisar 

    if (argc >= 2) {
        src = argv[1];
    } else {
        printf("Digite uma expressao (ex: 2+3*(4-1)):\n> ");
        if (!fgets(buf, (int)sizeof(buf), stdin)) return 1;
        buf[strcspn(buf, "\r\n")] = 0;
        src = buf;
    }

    long resultado = 0;
    char msg_erro[128];
    if (!analisar_expressao(src, &resultado, msg_erro, sizeof(msg_erro))) {
        fprintf(stderr, "Erro: %s\n", msg_erro);
        fprintf(stderr, "Entrada: %s\n", src);
        return 1;
    }

    printf("%ld\n", resultado); 
    return 0;              
}

