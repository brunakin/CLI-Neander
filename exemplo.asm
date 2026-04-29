; Soma A + B e guarda em RESULT

ORG 0

LDA A
ADD B
STA RESULT
HLT

; dados
A      DATA 2
B      DATA 3
RESULT SPACE 1

