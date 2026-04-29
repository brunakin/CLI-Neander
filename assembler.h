#ifndef NEANDER_ASSEMBLER_H
#define NEANDER_ASSEMBLER_H

#include <stddef.h>

int montar_arquivo(const char* caminho_asm,
                   const char* caminho_mem,
                   char* msg_erro,
                   size_t tam_msg_erro);

#endif

