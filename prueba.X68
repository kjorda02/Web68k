        ORG    $1000

START:  ;MOVE.L  #42,D0
        ;MOVE.W  #12,D1
        ;TRAP    #15
        ;RTS
        MOVE.W  #4,42(A0)

        END START
