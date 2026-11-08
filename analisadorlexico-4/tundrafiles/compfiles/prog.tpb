INPP
AMEM 1
DSVS MAIN

P:
ENPR 1
AMEM 1              ; s
DSVS P_CORPO

F:
ENPR 2
CRVL 2,-4           ; z
CRCT 0
CMME                ; z < 0
DSVF F1
DSVR L200,1,2       ; goto 200: rótulo no nível 1, saindo do nível 2

F1:
CRVL 2,-4
CRCT 0
CMIG                ; z = 0
DSVF F2
CRCT 2
ARMZ 2,-5           ; f := 2
DSVS F_FIM

F2:
AMEM 1              ; espaço do resultado de f(z-2)
CRVL 2,-4
CRCT 2
SUBT                ; z-2
CHPR F,2            ; chamador (f) está no nível 2
CRVL 2,-4
MULT
CRCT 1
SOMA
ARMZ 2,-5           ; f := f(z-2)*z + 1

F_FIM:
RTPR 2,1

P_CORPO:
L100:
AMEM 1              ; resultado de f(t)
CRVI 1,-4           ; valor de t (parâmetro por referência)
CHPR F,1            ; chamador (corpo de p) está no nível 1
ARMZ 1,0            ; s := f(t)
CRVL 1,0
ARMI 1,-4           ; t := s
CRVI 1,-4
CRVL 0,0
CMME                ; t < x
DSVF L200
DSVS L100

L200:
ENRT 1,1            ; s := D[1]+1-1
CRVL 0,0
CRCT 1
SUBT
ARMZ 0,0            ; x := x-1
DMEM 1              ; libera s
RTPR 1,1

R:
ENPR 1
DSVS R_CORPO

Q:
ENPR 2
AMEM 1              ; y
LEIT
ARMZ 2,0
CREN 2,0            ; endereço de y
CHPR P,2            ; chamador (q) está no nível 2
CRVL 2,0
IMPR
DMEM 1
RTPR 2,0

R_CORPO:
CHPR Q,1            ; chamador (r) está no nível 1
RTPR 1,0

MAIN:
LEIT
ARMZ 0,0
CHPR R,0            ; chamador (programa principal) está no nível 0
DMEM 1
PARA