; BRA TEST
        ORG    $1000
START:  ADD.W  #1,D0
        BRA     START
        END START

; ORI TO SR TEST
;         ORG    $1000
; START:  ORI #$8818,SR
;         END START

; ORI TO CCR TEST
;         ORG    $1000
; START:  ORI #$18,CCR
;         END START

; OR TEST
;         ORG    $1000
; START:  MOVE.W #$02A3,D0
;         OR.W    #$3146,D0
;         END START

; SUBI TEST
;         ORG    $1000
; TAG:    DS.L    1
; START:  SUB.L  #70000,TAG
;         MOVE.L  (TAG),D0
;         END START

; ADDI TEST
;         ORG    $1000
; TAG:    DS.L    1
; START:  ADD.L  #70000,TAG
;         MOVE.L  (TAG),D0
;         END START