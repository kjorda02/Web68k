; TEST PROGRAM 1: CALCULATE FIB(N)
        ORG    $1000
N:      DC.W    9
START:
        MOVE.L  #0,D0
        MOVE.L  #1,D1
        MOVE.W  (N),D7
        SUB.W   #3,D7

LOOP:
                MOVE.L  D0,D2
                ADD.L   D1,D2
                MOVE.L  D1,D0
                MOVE.L  D2,D1
        DBRA    D7,LOOP
        END START

; BRA TEST 2
;         ORG    $1000
; START:
;         BRA START
;         END START

; BRA TEST 2
;         ORG    $1000
; START:  MOVE.W  #4,D0
;         MOVE.W  #-1,D1
;         CMP.W   D0,D1
;         BHI     START
;         END START

; BRA TEST
;         ORG    $1000
; START:  ADD.W  #1,D0
;         BRA     START
;         END START

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

; OVERFLOW TEST 1
;         ORG    $1000
; START:  MOVE.B  #$7F,D0
;         ADD.B   #1,D0

; OVERFLOW TEST 2
;         ORG    $1000
; START:  MOVE.B  #$80,D0
;         SUB.B   #1,D0

; CARRY TEST 1
;         ORG    $1000
; START:  MOVE.B  #$FF,D0
;         ADD.B   #1,D0

; CARRY TEST 2
;         ORG    $1000
; START:  MOVE.B  #0,D0
;         SUB.B   #1,D0