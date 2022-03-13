;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;  G  --  crt0.asm
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  Copyright (c) 1993-1995 Jeremy Hall <jah@ilena.demon.co.uk>
;
;  Startup code for `G' editor for 16-bit real mode DOS and 386+ CPU's
;
;  Should be assembled using: wasm crt0 -bt=DOS -ml -2r_
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        name    cstart

        extrn   main_                   : far
        extrn   __InitRtns              : far
        extrn   _edata                  : byte  ; end of DATA (start of BSS)
        extrn   _end                    : byte  ; end of BSS (start of STACK)
        extrn   "C",__curbrk            : word
        extrn   "C",__psp               : word
        extrn   __no87                  : word
        extrn   "C",__LpCmdLine         : word
        extrn   __close_ovl_file        : word
        extrn   ____Argv                : word

STACK_SIZE equ  2000h

DGROUP  group CONST,XIB,XI,XIE,STACK

_TEXT   segment word public 'CODE'
CONST   segment word public 'DATA'
public   __8087cw
__8087cw  dw  137fh
CONST   ends
XIB     segment word public 'DATA'
XIB     ends
XI      segment word public 'DATA'
XI      ends
XIE     segment word public 'DATA'
XIE     ends
STACK   segment para stack  'STACK'
        db      (STACK_SIZE) dup(?)
STACK   ends

        public  _cstart_
        public  _Not_Enough_Memory_
        public  _matherr_

        assume  nothing
        assume  cs:_TEXT
        assume  es:DGROUP

_cstart_ proc near
        sti                             ; enable interrupts
        mov     dx,DGROUP               ; get proper stack segment
        mov     es,dx                   ; point to data segment
        mov     bx,offset DGROUP:_end   ; get bottom of stack
        add     bx,0Fh                  ; ...
        and     bl,0F0h                 ; ...
        mov     si,bx                   ; save bottom of stack
        mov     es:__psp,ds             ; save segment address of PSP

        add     bx,sp                   ; calculate top address for stack
        mov     ss,dx                   ; set stack segment
        mov     sp,bx                   ; set sp relative to DGROUP
        mov     es:__curbrk,bx          ; set top of memory owned by process
        shr     bx,4                    ; calc # of paragraphs needed for DS
        mov     cx,ds:2h                ; get highest segment address
        sub     cx,dx                   ; calc # of paragraphs available
        cmp     bx,cx                   ; compare with what we need
        jb      enuf                    ; if not enough memory
_Not_Enough_Memory_:
        mov     al,1
        jmp     __do_exit_with_msg__
;
;  free up memory beyond the end of the stack
;
enuf:   add     bx,dx                   ; plus start of data segment
        mov     di,ds                   ; di = es:__psp,
        sub     bx,di                   ; calc # of para's we want to keep
        mov     ah,4ah                  ; "SETBLOCK" func
        int     21h                     ; free the memory
;
;  copy command line into bottom of stack
;
        mov     bx,si
        mov     es:__LpCmdLine+0,si     ; stash lpCmdLine pointer
        mov     es:__LpCmdLine+2,es     ; ...
        mov     es,di                   ; point es to PSP
        mov     di,81H                  ; DOS command buffer __psp:80
        xor     ch,ch                   ; get length of command
        mov     cl,-1[di]
        cld                             ; set direction forward
        mov     al,' '
        rep     scasb
        lea     si,-1[di]
        mov     es,dx                   ; restore es
        mov     di,bx                   ; restore bottom of stack to di
        je      noparm
        inc     cx
        rep     movsb
noparm: db      066h,031h,0c0h          ; xor eax,eax
        mov     __no87,ax               ; set state of "NO87" environment var
        stosw

        assume  ds:DGROUP
        mov     ds,dx
        mov     cx,offset DGROUP:_end   ; end of _BSS segment (start of STACK)
        mov     di,offset DGROUP:_edata ; start of _BSS segment
        sub     cx,di                   ; calc # of bytes in _BSS segment
        shr     cx,2                    ; / 4 for stosd, ax still zero
        db      0f3h,066h,0abh          ; stosd

        mov     __close_ovl_file,offset _matherr_
        mov     __close_ovl_file+2,cs   ; - ...

        mov     ax,0FFh                 ; run all initalizers
        call    __InitRtns              ; call initializer routines
        mov     bx,ss:____Argv
        mov     cx,ss:____Argv+2H
        jmp     main_
_cstart_ endp

__exit_ proc near
        public  "C",__exit_
        public  __do_exit_with_msg__
__do_exit_with_msg__:
        mov     ah,04cH                 ; DOS call to exit with return code
        int     021h                    ; back to DOS, al has return code
__exit_ endp

public  __GETDS
__GETDS proc    near
        ret
__GETDS endp

_matherr_ proc far
        ret
_matherr_ endp

_TEXT   ends
        end     _cstart_
