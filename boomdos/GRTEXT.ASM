;//////////////////////////////////////////////////////////////////////////
;//                                                                      //
;//   BOOM 2 Engine                                                      //
;//                                                                      //
;//   grtext.asm - string writting implementation                        //
;//                                                                      //
;//   by Ivaylo Beltchev                                                 //
;//   e-mail: ivob@geocities.com                                         //
;//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
;//                                                                      //
;//////////////////////////////////////////////////////////////////////////

                .386p
                .model  flat

                .code
; PUBLICS
; ~~~~~~~
                public  textout_asm_
                public  _scr_cols
                public  _scr_bpp

                .data
_scr_cols       dword   0
_scr_bpp        dword   0
; 5x5 font by Emil Dotchevski
font            db      0,00000000b,00000000b,00000000b,00000000b,00000000b,0 ;_
                db      0,01000000b,01000000b,01000000b,00000000b,01000000b,0 ;!
                db      0,01010000b,01010000b,00000000b,00000000b,00000000b,0 ;"
                db      0,01010000b,11111000b,01010000b,11111000b,01010000b,0 ;#
                db      0,11111000b,10100000b,11111000b,00101000b,11111000b,0 ;$
                db      0,11001000b,00010000b,00100000b,01000000b,10011000b,0 ;%
                db      0,00100000b,01010000b,01100000b,10011000b,01111000b,0 ;&
                db      0,01000000b,01000000b,00000000b,00000000b,00000000b,0 ;'
                db      0,00110000b,01000000b,01000000b,01000000b,00110000b,0 ;(
                db      0,01100000b,00010000b,00010000b,00010000b,01100000b,0 ;)
                db      0,10101000b,01110000b,11111000b,01110000b,10101000b,0 ;*
                db      0,00100000b,00100000b,11111000b,00100000b,00100000b,0 ;+
                db      0,00000000b,00000000b,01000000b,01000000b,10000000b,0 ;,
                db      0,00000000b,00000000b,11111000b,00000000b,00000000b,0 ;-
                db      0,00000000b,00000000b,00000000b,01000000b,00000000b,0 ;.
                db      0,00001000b,00010000b,00100000b,01000000b,10000000b,0 ;/
                db      0,11111000b,10001000b,10001000b,10001000b,11111000b,0 ;0
                db      0,00100000b,00100000b,00100000b,00100000b,00100000b,0 ;1
                db      0,11111000b,00001000b,11111000b,10000000b,11111000b,0 ;2
                db      0,11111000b,00001000b,01111000b,00001000b,11111000b,0 ;3
                db      0,10001000b,10001000b,11111000b,00001000b,00001000b,0 ;4
                db      0,11111000b,10000000b,11111000b,00001000b,11111000b,0 ;5
                db      0,11111000b,10000000b,11111000b,10001000b,11111000b,0 ;6
                db      0,11111000b,00001000b,00001000b,00001000b,00001000b,0 ;7
                db      0,11111000b,10001000b,11111000b,10001000b,11111000b,0 ;8
                db      0,11111000b,10001000b,11111000b,00001000b,11111000b,0 ;9
                db      0,00000000b,01000000b,00000000b,01000000b,00000000b,0 ;:
                db      0,01000000b,00000000b,01000000b,01000000b,10000000b,0 ;;
                db      0,00010000b,00110000b,01110000b,00110000b,00010000b,0 ;<
                db      0,00000000b,11111000b,00000000b,11111000b,00000000b,0 ;=
                db      0,00100000b,00110000b,00111000b,00110000b,00100000b,0 ;>
                db      0,00000000b,00100000b,01110000b,11111000b,00000000b,0 ;?
                db      0,00000000b,11111000b,01110000b,00100000b,00000000b,0 ;@
                db      0,11111000b,10001000b,11111000b,10001000b,10001000b,0 ;A
                db      0,11110000b,10001000b,11110000b,10001000b,11110000b,0 ;B
                db      0,11111000b,10000000b,10000000b,10000000b,11111000b,0 ;C
                db      0,11110000b,10001000b,10001000b,10001000b,11110000b,0 ;D
                db      0,11111000b,10000000b,11110000b,10000000b,11111000b,0 ;E
                db      0,11111000b,10000000b,11110000b,10000000b,10000000b,0 ;F
                db      0,11111000b,10000000b,10111000b,10001000b,11111000b,0 ;G
                db      0,10001000b,10001000b,11111000b,10001000b,10001000b,0 ;H
                db      0,01110000b,00100000b,00100000b,00100000b,01110000b,0 ;I
                db      0,00111000b,00010000b,00010000b,00010000b,11100000b,0 ;J
                db      0,10001000b,10010000b,11100000b,10010000b,10001000b,0 ;K
                db      0,10000000b,10000000b,10000000b,10000000b,11111000b,0 ;L
                db      0,10001000b,11011000b,10101000b,10001000b,10001000b,0 ;M
                db      0,10001000b,11001000b,10101000b,10011000b,10001000b,0 ;N
                db      0,11111000b,10001000b,10001000b,10001000b,11111000b,0 ;O
                db      0,11111000b,10001000b,11111000b,10000000b,10000000b,0 ;P
                db      0,11111000b,10001000b,10001000b,10011000b,11111000b,4 ;Q
                db      0,11111000b,10001000b,11111000b,10100000b,10011000b,0 ;R
                db      0,11111000b,10000000b,11111000b,00001000b,11111000b,0 ;S
                db      0,11111000b,00100000b,00100000b,00100000b,00100000b,0 ;T
                db      0,10001000b,10001000b,10001000b,10001000b,11111000b,0 ;U
                db      0,10001000b,10001000b,10001000b,01010000b,00100000b,0 ;V
                db      0,10001000b,10001000b,10101000b,11011000b,10001000b,0 ;W
                db      0,10001000b,01010000b,00100000b,01010000b,10001000b,0 ;X
                db      0,10001000b,01010000b,00100000b,00100000b,00100000b,0 ;Y
                db      0,11111000b,00010000b,00100000b,01000000b,11111000b,0 ;Z

txout           dd      0

; Description:
;   write string
; Entry:
;   edi - address
;   esi - pointer to the string
;   edx - color
                .code
textout_asm_    proc
                push    ebp
                mov     ebp,_scr_cols
                mov     eax,_scr_bpp
                lea     ebx,txout8
                cmp     eax,8
                je      toc1
                add     ebp,ebp
                lea     ebx,txout16
                cmp     eax,16
                je      toc1
                add     ebp,_scr_cols
                lea     ebx,txout24
                cmp     eax,24
                je      toc1
                add     ebp,_scr_cols
                cmp     eax,32
                jne     toqq
toc1:           mov     txout,ebx
toml:
                xor     eax,eax
                mov     al,[esi]
                or      eax,eax
                jz      toqq
                sub     eax,' '
                lea     ebx,font[eax*8]
                sub     ebx,eax
                mov     ch,8
                call    [txout]
                lea     eax,[8*ebp]
                sub     edi,eax
                inc     esi
                jmp     toml
toqq:
                pop     ebp
                ret

txout8:
                mov     al,[ebx]
                mov     cl,8
to8l2:
                shl     al,1
                jnc     to8c1
                mov     [edi],dl
to8c1:
                inc     edi
                dec     cl
                jne     to8l2
                inc     ebx
                lea     edi,[edi+ebp-8]
                dec     ch
                jne     txout8
                add     edi,8
                ret

txout16:
                mov     al,[ebx]
                mov     cl,8
to16l2:
                shl     al,1
                jnc     to16c1
                mov     [edi],dx
to16c1:
                add     edi,2
                dec     cl
                jne     to16l2
                inc     ebx
                lea     edi,[edi+ebp-16]
                dec     ch
                jne     txout16
                add     edi,16
                ret

txout24:
                mov     al,[ebx]
                mov     cl,8
to24l2:
                shl     al,1
                jnc     to24c1
                and     dword ptr [edi],0FF000000h
                or      [edi],edx
to24c1:
                add     edi,3
                dec     cl
                jne     to24l2
                inc     ebx
                lea     edi,[edi+ebp-24]
                dec     ch
                jne     txout24
                add     edi,24
                ret

txout32:
                mov     al,[ebx]
                mov     cl,8
to32l2:
                shl     al,1
                jnc     to32c1
                mov     [edi],edx
to32c1:
                add     edi,4
                dec     cl
                jne     to32l2
                inc     ebx
                lea     edi,[edi+ebp-32]
                dec     ch
                jne     txout32
                add     edi,32
                ret
textout_asm_    endp

                end
