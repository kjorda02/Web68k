        ORG    $1000

DATA:   DS.W    1
START:  MOVE.W   #80,(DATA)
        ADDA.W   (DATA),A0

        END START