#include "assembler.h"

#include <stdio.h> 

int main(int argc, char** argv) {
    
    if (argc < 3) {
        printf("Uso: %s <entrada.asm> <saida.mem>\n", argv[0]);
        return 1;
    }

    char msg[256];
    if (!montar_arquivo(argv[1], argv[2], msg, sizeof(msg))) {
        printf("Erro: %s\n", msg);
        return 1;
    }

    printf("OK: gerado %s\n", argv[2]);
    return 0;
}

