        ORG $1000
START:
        MOVE.L  #$f200,D0
        TAS     D0
        END START

; SUBROUTINE TEST #2 (PASSED)
*---------------------------------------------------------------------
* Program : Example subroutine 2. Library subroutine with use of stack
* Description: SUM.W = sum the components of a vector, VEC, of N words
*---------------------------------------------------------------------
;         ORG     $1000
;         
; SUMVEC:
;         MOVE.L D0,-(A7)         ; save value of D0 in the stack
;         MOVE.L D1,-(A7)         ; save value of D1 in the stack
;         MOVE.L A0,-(A7)         ; save value of A0 in the stack
;         
;         SUBQ.W #2,A7 ; reserve space for a 1-word local 
;         
;         MOVE.W 22(A7),D0 ; get 1st input parameter (#N)
;         MOVE.L 18(A7),A0 ; get 2nd input parameter (@VEC)
;         
;         CLR.W (A7) ; initialize local variable
;         SUBQ.W #1,D0 ; adjust loop counter
;         SUMLOOP:
;         MOVE.W (A0)+,D1 ; get vector component and point to next one
;         ADD.W D1,(A7) ; accumulate addition in local variable
;         DBRA D0,SUMLOOP
;         
;         MOVE.W (A7),24(A7) ; transfer result to the output parameter
;         
;         ADDQ.W #2,A7
;         
;         MOVE.L (A7)+,A0 ; restore value of A0
;         MOVE.L (A7)+,D1 ; restore value of D1
;         MOVE.L (A7)+,D0 ; restore value of D0
;         
;         RTS
;         
; N       EQU 5           ; vector length
; VEC     DC.W 1,-3,3,4,100
; SUM     DS.W 1
; START:
;         LEA     VEC,A0
;         
;         MOVE.W  #0,-(A7)        ; reserve space for output parameter and initia.
;         MOVE.W  #N,-(A7)        ; reserve space and initialize input parameters
;         MOVE.L  A0,-(A7)
;         
;         JSR SUMVEC              ; call to subroutine
;         
;         ADDQ.L #6,A7            ; discard the two input parameters from stack
;         MOVE.W (A7)+,SUM        ; get the output parameter and discard it
;         
;         MOVE.W  SUM,D2
        

; SUBTROUTINE TEST #1 (PASSED)
*---------------------------------------------------------------------
* Program: Example subroutine 1
* Description: Z.W = (sum of N first natural numbers)
* + (sum of M first natural numbers)
* - (sum of N + M first natural numbers)
*---------------------------------------------------------------------
;         ORG $1000
; SUMATORIO:
;         MOVE.W #0,D1
;         ET0:
;         ADD.W D0,D1
;         DBRA D0,ET0
;         RTS
;         
; N       EQU -3
; M       EQU -8
; Z       DS.W 1
; START:
;         MOVE.W #N,D0
;         JSR SUMATORIO
;         MOVE.W D1,Z
;         
;         MOVE.W #M,D0
;         JSR SUMATORIO
;         ADD.W D1,Z
;         
;         MOVE.W #N,D0
;         ADD.W #M,D0
;         JSR SUMATORIO
;         SUB.W D1,Z
;         
;         MOVE.W  Z,D2
;         
;         END     START