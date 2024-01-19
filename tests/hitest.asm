;****************************************************************************
;*                                                                          *
;*  HITEST.ASM -                                            Chip Anderson   *
;*                                                                          *
;*      DOS XMS Driver Testing Program                                      *
;*                                                                          *
;*      Used to verify the functionality of any DOS XMS Driver.             *
;*                                                                          *
;****************************************************************************

        name    Test
        title   'DOS XMS Test Program'

code    segment byte public 'CODE'

        assume  cs:code, ds:code, es:code

        org     100h

main    proc    near

        mov     ah,9h
        mov     dx,offset SignOn
        int     21h

        ; Is an XMS Driver installed?
        mov     ax,4300h
        int     2Fh
        cmp     al,80h
        je      HMMIn
        mov     ah,9h
        mov     dx,offset AintThere
        int     21h
        int     20h                     ; Terminate

HMMIn:  mov     ah,9h
        mov     dx,offset FoundOne
        int     21h

        ; Get the HMM's control entry point
        mov     ax,4310h
        int     2Fh
        mov     word ptr cs:[HMMEntryPt][0],bx
        mov     word ptr cs:[HMMEntryPt][2],es

        ; Get the HMM's version number
        mov     ax,0
        call    cs:[HMMEntryPt]
        
        push    bx                      ; Save driver internal number
        call    PrintAX
        
        mov     ah,9h
        mov     dx,offset InternalVer
        int     21h        
        
        pop     ax                      ; Restore driver internal number
        
        call    PrintAX

        
        ;*------------------------------------------------------------------*
        ;*      Basic Function Check                                        *
        ;*------------------------------------------------------------------*

StartTesting:
        xor     bx,bx
        push    cs
        pop     es
        mov     si,offset rgbCommands
cloop:  mov     bl,byte ptr es:[si]
        cmp     bl,'$'
        je      exit

        ; Print the command being executed
        mov     ch,bl
        shl     bx,1
        mov     dx,[rgszCommands+bx]
        mov     ah,9h
        int     21h

        cmp     ch,cmdDivider
        je      isnop
        
        push    bx
	mov	ah,ch
	mov	dx,8192
        call    cs:[HMMEntryPt]
        pop     bx

        ; Print the result
        or      ax,ax
        jz      Fail
        mov     dx,offset Success
        jmp     short PrintIt
Fail:   mov     dx,offset Failure
PrintIt:mov     ah,9
        int     21h

        ; Now print the state of the A20 Line
        mov     dx,offset A20Msg
        int     21h
        mov     ah,7h
        call    cs:[HMMEntryPt]
        or      ax,ax
        jz      NoA20
        mov     dx,offset A20On
        jmp     short PrntIt2
NoA20:  mov     dx,offset A20Off
PrntIt2:mov     ah,9h
        int     21h

isnop:  inc     si
        jmp     cloop

exit:   ; Now do the extended memory test
        mov     dx,[rgszCommands+16]
        mov     ah,9h
        int     21h
        mov     ah,08h
        call    cs:[HMMEntryPt]
        call    PrintAX
        
        ; Allocate alot of stuff
        mov     dx,[rgszCommands+18]
        mov     ah,9h
        int     21h
        mov     dx,100h
        mov     ah,09h
        call    cs:[HMMEntryPt]
        push    dx
        push    dx    
        call    PrintAX
        pop     ax
        call    PrintAX
                
        mov     dx,[rgszCommands+16]
        mov     ah,9h
        int     21h
        mov     ah,08h
        call    cs:[HMMEntryPt]
        call    PrintAX
        
        mov     dx,[rgszCommands+20]
        mov     ah,9h
        int     21h
        pop     dx
        mov     ah,10
        call    cs:[HMMEntryPt]
        call    PrintAX
        
        mov     dx,[rgszCommands+16]
        mov     ah,9h
        int     21h
        mov     ah,08h
        call    cs:[HMMEntryPt]
        call    PrintAX
        
        ret

main    endp


PrintAX proc    near
        ; Print it
        mov     bx,ax
        mov     ch,4
HexLoop:mov     cl,4
        rol     bx,cl
        mov     al,bl
        and     al,0Fh
        add     al,30h
        cmp     al,3Ah
        jl      Output
        add     al,07h
Output: mov     dl,al
        mov     ah,02h
        int     21h
        dec     ch
        jnz     HexLoop
        ret
        
PrintAX endp

;*--------------------------------------------------------------------------*
;*      Data Area                                                           *
;*--------------------------------------------------------------------------*

cmdVersion  equ     0
cmdRequest  equ     1
cmdRelease  equ     2
cmdGEnable  equ     3
cmdGDisable equ     4
cmdTEnable  equ     5
cmdTDisable equ     6
cmdA20Query equ     7
cmdExtQuery equ     8
cmdExtAlloc equ     9
cmdExtFree  equ     10
cmdDivider  equ     11

rgbCommands db      cmdDivider

            ; Normal High Memory Area Test
            db      cmdRequest, cmdRelease, cmdDivider

            ; Nested High Memory Area Test
            db      cmdRequest, cmdRequest, cmdRelease, cmdRelease, cmdDivider
            db      cmdDivider

	    ; Global vs Local A20 Test
	    db	    cmdGEnable
	    db		cmdTEnable, cmdTDisable
	    db	    cmdGDisable, cmdDivider

	    ; Local vs Global A20 Test
	    db	    cmdTEnable
	    db		cmdGEnable, cmdGDisable
	    db	    cmdTDisable, cmdDivider

            db      '$'

rgszCommands dw     szVer
            dw      szHighReq
            dw      szHighRel
            dw      szGEnable
            dw      szGDisable
            dw      szTEnable
            dw      szTDisable
            dw      szTestA20
            dw      szExtQuery
            dw      szExtAlloc
            dw      szExtFree
            dw      szDivider

szVer       db      13,10,'Version: $'
szHighReq   db      13,10,'Request High Memory Area:  $'
szHighRel   db      13,10,'Release High Memory Area:  $'
szGEnable   db      13,10,'Globally Enable A20 Line:  $'
szGDisable  db      13,10,'Globally Disable A20 Line: $'
szTEnable   db      13,10,'Temp. Enable A20 Line:     $'
szTDisable  db      13,10,'Temp. Disable A20 Line:    $'
szTestA20   db      13,10,'See if the A20 Line is On: $'
szExtQuery  db      13,10,'Extended Memory Free:      $'
szExtAlloc  db      13,10,'Allocating 100K:           $'
szExtFree   db      13,10,'Freeing it:                $'
szDivider   db      13,10,'------------------------------------------------------$'

Success     db      'Succeeded$'
Failure     db      'Failed   $'

A20Msg      db      ' - A20 Line is $'
A20On       db      'On$'
A20Off      db      'Off$'

SignOn      db      13,10,'High Memory Manager Test Program 2.0 - 7/05/88'
            db      13,10,'Copyright 1988, Microsoft Corp.'
            db      13,10,'$'

FoundOne    db      13,10,'High Memory Manager is Installed - Version $'
InternalVer db      13,10,'                                   Internal Version $'
AintThere   db      13,10,'High Memory Manager not Installed.$'

HMMEntryPt  dd      ?

code    ends

        end     main
