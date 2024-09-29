        ORG    $1000
TAG     DS.W    1

START:  ;MOVE.L  #42,D0
        ;MOVE.W  #12,D1
        ;TRAP    #15
        ;RTS
        ;MOVE.W  #3,A0
        MOVEA.W #45,A0
        MOVE.B #18,-34(A0)

        END START
