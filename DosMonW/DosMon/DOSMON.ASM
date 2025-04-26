
;*** DOS (int 21h) monitor - dll

	.286
	.MODEL SMALL
	.386
	option casemap:none

;--- ?USEHWBREAK:
;--- 1: use either exception 01 or toolhelp's interruptregister
;---    pro: no need to modify protected-mode vectors
;---    con: may not work for NoHookDosCall, since address has a GDT selector.
;--- 0: protected-mode vectors are modified (int 21h/GetSetKernelDosProc)

?USEHWBREAK equ 0           ;1=use hardware breakpoint (int 31h, ax=0b00h/0b01h)

if ?USEHWBREAK
?USEINTERRUPTREGISTER equ 0 ;1=use ToolHelp InterruptRegister()
endif

	include winproto.inc
	include dosbase.inc

	include dosmon.inc

PMTYPE = 0
RMTYPE = 1
NHTYPE = 2

CStr macro text:vararg
local sym
	.const
sym db text,0
	.code
	exitm <addr sym>
endm

	.CODE

;pmcallback dd ?             ;not used currently

oldint21p  dd ?             ;PM: (old) vector, either int 21h pm or kerneldosproc
phook      dd ?             ;PM: far16 address of function to call
pmstack    dw ?             ;PM: SP 
dsreg      dw ?             ;PM: DS (=DGROUP) register
oldintr    dd ?             ;RM: old rm vector
rhook      dd ?             ;RM: address of table to store registers
codeseg    dw ?             ;RM: code segment
wCSalias   dw ?             ;CS alias
oldbase    dd 0             ;original address if code was moved to conv memory

if ?USEHWBREAK
oldexc01   dd ?
oldpketvek dd ?
hwhandle   dw 0
 if ?USEINTERRUPTREGISTER
hTask      dw 0
 endif
endif

rflag      db 1             ;1=store registers  (constant)

        align 2

regstruc   ZREGS <>
iflag      dw PMTYPE,RMTYPE,NHTYPE

UnregisterDosMon proto far pascal intnr:word,atype:word

;*** windows main procedure (DLL)

LibEntry proc far pascal uses si di

local   toldbase:dword

        invoke GetSelectorBase, cs               ;we must be in low memory
        test    dx,0FFF0h
        jz      ind1
        mov     word ptr toldbase+0,ax
        mov     word ptr toldbase+2,dx

        mov     cx,cs
        lsl     ax,cx
        jnz     ind1
        inc     ax
        mov     si,ax

        invoke  GlobalDosAlloc,ax
        and     ax,ax
        jz      ind1

        mov     es,ax
        push    ds
        push    cs
        pop     ds
        mov     cx,si
        xor     si,si
        xor     di,di
        rep     movsb
        pop     ds

        mov     es:[wCSalias],ax
        mov     eax,toldbase
        mov     es:[oldbase],eax

        movzx   ecx,dx             ;real mode segment in dx
        shl     ecx,4
        invoke SetSelectorBase, cs, ecx

        jmp     ind2
ind1:
        invoke  AllocCStoDSAlias,cs
        mov     es,ax
        mov     es:[wCSalias],ax
ind2:
        mov     ax,1
        ret
LibEntry endp

;*** windows exit procedure (DLL)

Wep     proc far pascal uses si wCode:word

        mov     bx,offset iflag
        mov     cx,3
@@:
        pusha
        mov     ax,cs:[bx]
        .if ah
            mov ah,0
            invoke  UnregisterDosMon,21h,ax
        .endif
        popa
        add     bx,2
        loop    @B

        mov     eax,cs:[oldbase]
        and     eax,eax
        jz      wep2
        push    cs:[wCSalias]
        invoke SetSelectorBase, cs, eax
        pop     ax
        invoke  GlobalDosFree,ax
        jmp     wep3
wep2:
        invoke  FreeSelector,cs:[wCSalias]
wep3:
        mov     ax,1
        ret
Wep     endp

;*** filename abspeichern (RM + PM)

getfn   proc near c

        push    ds
        push    si
        mov     di,[si.ZREGS.pdx]
        mov     es,[si.ZREGS.ods]
        mov     al,0
        mov     cx,128
        repnz   scasb
        jnz     getfnex      ;keine 0 entdeckt
        sub     cx,128
        neg     cx
@@:
        dec     di
        mov     al,es:[di]
        cmp     al,'\'
        jz      @F
        cmp     al,':'
        jz      @F
        loop    @B
        dec     di
@@:
        inc     di
        lea     si,[si.ZREGS.res1]
        xchg    si,di
        mov     ax,ds
        mov     cx,es
        mov     es,ax
        mov     ds,cx
        mov     cx,12
@@:
        mov     al,[si]
        movsb
        and     al,al
        jz      @F
        loop    @B
        mov     al,0
@@:
        rep     stosb
getfnex:
        pop     si
        pop     ds
        ret
getfn   endp

;*** store registers (real-mode)

rregout proc near c

        test    byte ptr cs:[rflag],01
        jz      rsm111
                                       ;ip = bp + 20
        pusha                          ;bp+4 - bp + 18
        push    ds                     ;bp+2
        push    es                     ;bp+0

        push    ds
        push    si
                                       ;real mode
        lds     si,cs:[rhook]
        push    ax
        mov     ax,[si]
        inc     word ptr [si+0]
        add     word ptr [si+4],1
;        jnc     @F
;        inc     word ptr [si+6]
;@@:
        cmp     ax,[si+2]
        jb      @F
        mov     word ptr [si+0],0
@@:
        mov     si,size ZREGS
        mul     si
        add     ax,0010h
        mov     si,ax
        pop     ax

        @saveall
        mov     word ptr [si.ZREGS.user1],0
        mov     al,byte ptr [si.ZREGS.pax+1]
        @chkfn

        pop     es
        pop     ds
        popa
rsm111:
        ret
rregout  endp

;*** store registers (protected-mode)

pregout proc near c

        test    byte ptr cs:[rflag],01
        jz      sm111
                                       ;ip = bp + 20
        pusha                          ;bp+4 - bp + 18
        push    ds                     ;bp+2
        push    es                     ;bp+0

        push    ds
        push    si

        mov     ds,cs:[wCSalias]
        mov     si,offset regstruc
        @saveall
        mov     word ptr [si.ZREGS.user1],0
        mov     al,byte ptr [si.ZREGS.pax+1]
        @chkfn
                                         ;jetzt den Call ausfuehren
        mov     ax,ds
        mov     ds,cs:[dsreg]
        mov     dx,ds
        mov     cx,sp
        mov     bx,ss
        mov     ss,dx
        mov     sp,cs:[pmstack]
        push    bx
        push    cx
        push    ax
        push    si
        call    dword ptr cs:[phook]
        pop     cx
        pop     bx
        mov     ss,bx
        mov     sp,cx
        pop     es
        pop     ds
        popa
sm111:
        ret
pregout endp

;*** Interrupt 0x21 real-mode

rintrou  proc far
        push    ax
        push    bx
        push    cx
        push    dx

        pushf                ;nur zur simulation eines INT

        cmp     ah,DOS_kill
        jz      rtermv
        cmp     ah,DOS_terminate
        jz      rtermv
        cmp     ah,DOS_keep
        jz      rtermv
        cmp     ah,DOS_exec
        jz      rtermw
        call    dword ptr cs:[oldintr]
rsm2:
        pushf
        call    rregout
        push    bp
        mov     bp,sp
        push    ax
        mov     ax,[bp+2]       ;flags
        mov     [bp+16],ax
        pop     ax
        pop     bp
        add     sp,10
        iret
rtermv:                                 ;wenn aufruf nicht synchron -> sofort ausgeben
        call    rregout
        add     sp,10
        jmp     dword ptr cs:[oldintr]
rtermw:                                 ;exec program
        call    rregout
        call    dword ptr cs:[oldintr]
        push    bp
        mov     bp,sp
        mov     byte ptr [bp+9],0ffh   ;ax
        pop     bp
        jmp     rsm2

rintrou  endp

;*** Interrupt 0x21 protected-mode

pintrou proc far
        push    ax
        push    bx
        push    cx
        push    dx

        pushf                ;nur zur simulation eines INT

        cmp     ah,DOS_kill
        jz      termv
        cmp     ah,DOS_terminate
        jz      termv
        cmp     ah,DOS_keep
        jz      termv
        cmp     ah,DOS_exec
        jz      termw
        call    dword ptr cs:[oldint21p]
sm2:
        pushf
        call    pregout
        push    bp
        mov     bp,sp
        push    ax
        mov     ax,[bp+2]       ;flags
        mov     [bp+16],ax
        pop     ax
        pop     bp
        add     sp,10
        iret
termv:                                 ;wenn aufruf nicht synchron -> sofort ausgeben
        call    pregout
        add     sp,10
        jmp     dword ptr cs:[oldint21p]
termw:                                 ;exec program
        call    pregout
        call    dword ptr cs:[oldint21p]
        push    bp
        mov     bp,sp
        mov     byte ptr [bp+9],0ffh   ;ax
        pop     bp
        jmp     sm2

pintrou endp

if ?USEHWBREAK
 if ?USEINTERRUPTREGISTER

THFR struct
wOldBp dw ?
xTHIP  dw ?
xTHCS  dw ?
xAX    dw ?
xIntNr dw ?
xHdl   dw ?
union
xCSIP  dd ?
struct
xIP    dw ?
xCS    dw ?
ends
ends
xFL    dw ?
THFR ends

intrproc proc far
        push    bp
        mov     bp,sp

        cmp     [bp].THFR.xIntNr,1
        jnz     @F
        push    eax
        mov     eax,[bp].THFR.xCSIP
        cmp     eax,cs:[oldpketvek]
        jz      intrproc_1
        pop     eax
@@:
        pop     bp
        retf
intrproc_1:
        push    bx
        mov     bx,cs:[hwhandle]
        mov     ax,0B03h    ;clear watchpoint state
        int     31h
        pop     bx

        mov     ax,cs
        shl     eax,16
        mov     ax,offset pintrou
        mov     [bp].THFR.xCSIP,eax
        pop     eax
        mov     ax,[bp].THFR.xAX
        pop     bp
        add     sp,10
        iret
intrproc endp

 else

EXC01FR struct   ; exception stack frame
wOldBp  dw ?
xDPMIIP dw ?
xDPMICS dw ?
xDPMIER dw ?
union
xCSIP   dd ?
struct
xIP     dw ?
xCS     dw ?
ends
ends
xFL     dw ?
EXC01FR ends

;*** EXC 01 handler
;*** have to check if it's our hw bp

pintr01 proc far
        push    bp
        mov     bp,sp

        pushf
        push    eax
        mov     eax,[bp].EXC01FR.xCSIP
        cmp     eax,cs:[oldpketvek]     ;is it our location?
        jz      @F
        pop     eax
        popf
        pop     bp
        jmp     dword ptr cs:[oldexc01]
@@:
        push    bx
        mov     bx,cs:[hwhandle]
        mov     ax,0B03h                ;clear debug watchpoint state
        int     31h
        pop     bx

        mov     ax,cs
        shl     eax,16
        mov     ax,offset pintrou
        mov     [bp].EXC01FR.xCSIP,eax
        pop     eax
        popf
        pop     bp
        retf
pintr01 endp

 endif

;--- ensure RF is set in 32-bit eflags register

newproc:
        pushfd
        sub     sp,2*4
        push    bp
        mov     bp,sp
        push    eax
        movzx   eax,word ptr cs:[oldpketvek+0]
        mov     [bp+2+0*4],eax
        movzx   eax,word ptr cs:[oldpketvek+2]
        mov     [bp+2+1*4],eax
        or      byte ptr [bp+2+2*4+2],1   ;resume flag setzen
        pop     eax
        pop     bp
        iretd
endif

;*** enable monitor for real-mode;
;--- hw breakpoints cannot be used here, since they may not work in real-mode

rmreg  proc pascal intnr:word,hookproc:dword

local   rmseg:word
                                           ;1.: real mode segment address -> rmseg
        invoke  GetSelectorBase, cs
        mov     cx,ax
        xor     ax,ax                      ;rc=FALSE
        cmp     dx,0010h
        jae     rmreg1                     ;error: base not in conv. memory space
        test    cx,000Fh
        jnz     rmreg1                     ;error: base not at start of paragraph
        mov     ax,cx
        shr     ax,4
        shl     dx,12
        or      ax,dx
        mov     rmseg,ax
                                           ;2.: get real mode interrupt vector
        mov     bl, byte ptr intnr
        mov     ax, 200h
        int     31h

        mov     es,cs:[wCSalias]           ;use csalias to write into _TEXT
        assume  es:_TEXT
        mov     word ptr es:[oldintr+0],dx
        mov     word ptr es:[oldintr+2],cx
        mov     ax,word ptr hookproc+0     ;3.: hookproc will be interpreted as
        mov     word ptr es:[rhook+0],ax   ;    far ptr to table 
        mov     ax,word ptr hookproc+2
        mov     word ptr es:[rhook+2],ax
        mov     ax,rmseg
        mov     es:[codeseg],ax

;        push    ds
;        push    es
;        mov     ax,ds
;        mov     es,ax
;        mov     di,offset rmds            ;ES:DI -> rmcs
;        mov     si,offset introu1         ;DS:SI -> callback pm address
;        mov     ax,cs
;        mov     ds,ax
;        mov     ax,303h
;        int     31h                       ;get real mode callback
;        pop     es
;        pop     ds
;        mov     word ptr es:[pmcallback+0],dx
;        mov     word ptr es:[pmcallback+2],cx
                                           ;5.: set real mode vector
        mov     cx,rmseg
        mov     dx,offset rintrou
        mov     bl,byte ptr intnr
        mov     ax,201h
        int     31h
                                           ;6.: rc=TRUE
        mov     ax,1
rmreg1:
        ret
rmreg   endp

if ?USEHWBREAK
getgdtselbase proc uses si

local gdt[6]:byte

	mov si,bx
	xor bx,bx
	mov cx,1
	mov ax,0
	int 31h
	jc exit
	mov bx,ax
	sgdt [gdt]
	mov dx,word ptr [gdt+2]
	mov cx,word ptr [gdt+4]
	mov ax,7
	int 31h
	jc exit
	mov dx,-1
	mov cx,0
	mov ax,8
	int 31h
	jc exit
	mov es,bx
	and si,not 7
	mov al,es:[si+4]
	mov ah,es:[si+7]
	shl eax,16
	mov ax,es:[si+2]
	clc
exit:
	pushf
	and bx,bx
	jz @F
	push ax
	mov ax,1
	int 31h
	pop ax
@@:
	popf
	ret
getgdtselbase endp
endif

;--- enable monitor for protected-mode; wtype: 0=std protected-mode int 21h, 1=nohookdoscall

pmreg  proc pascal intnr:word,hookproc:dword,mystack:word,wtype:word

        cmp     wtype,0       ;std protected-mode int 21h?
        jz      setpmint

;--- just get the address of the current nohookdosproc - don't modify it!

        cli
        invoke  GetSetKernelDosProc,0
        push    dx
        push    ax
        invoke  GetSetKernelDosProc,dx::ax
        sti
        pop     dx
        pop     cx
        jmp     next
setpmint:
        mov     bl,byte ptr intnr
        mov     ax,204h		;get pm int vector
        int     31h
        jc      rim_err
next:
        mov     es,cs:[wCSalias]      ;use csalias to write into _TEXT
        assume  es:_TEXT
        mov     word ptr es:[oldint21p+0],dx
        mov     word ptr es:[oldint21p+2],cx

        mov     eax,hookproc
        mov     es:[phook],eax
        mov     ax,ds
        mov     es:[dsreg],ax
        mov     ax,mystack
        mov     es:[pmstack],ax

if ?USEHWBREAK
        push    dx
        mov     bx,cx
        mov     ax,6	;get base (may fail if no LDT selector!!!)
        int     31h
        push    cx
        push    dx
        pop     eax     ;base in eax
        pop     dx
        jnc     @F
        test    bx,4
        jnz     rim_err
        push    dx
        call    getgdtselbase  ;get base of bx in eax
        pop     dx
        jc      rim_err
@@:
        movzx   edx,dx
        add     eax,edx ;eax=linear address of [oldint21p] vector

        push    eax
        pop     cx
        pop     bx

        mov     dx,0001    ;hw execute type
        mov     ax,0B00h
        int     31h
        jc      rim_err

        mov     es,cs:[wCSalias]
        mov     es:[hwhandle],bx
 if ?USEINTERRUPTREGISTER
        invoke  GetCurrentTask
        mov     [hTask],ax
        invoke  InterruptRegister,ax,addr intrproc
 else
        mov     bl,1
        mov     ax,202h  ; get exception vector
        int     31h
        jc      rim_err
        mov     word ptr es:[oldexc01+0],dx
        mov     word ptr es:[oldexc01+2],cx
        mov     cx,cs
        mov     dx,offset pintr01
        @DPMI_SETEXCVEC 01
        jc      rim_err
 endif
;--- add a level of indirection to set RF
        mov     eax,es:[oldint21p]
        mov     es:[oldpketvek],eax
        mov     ax,cs
        shl     eax,16
        mov     ax,offset newproc
        mov     es:[oldint21p],eax
else
        cmp     wtype,0
        jz      @F
        invoke  GetSetKernelDosProc,pintrou
        jmp     next2
@@:
        mov     cx,cs
        mov     dx,offset pintrou
        mov     bl,byte ptr intnr
        mov     ax,205h    ; set pm int vector
        int     31h
        jc      rim_err
next2:
endif                                     ;rc=TRUE
        mov     ax,1
        ret
rim_err:
ifdef _DEBUG
        invoke  OutputDebugString, CStr("pmreg failed",13,10)
endif
        xor     ax,ax
        ret
pmreg   endp
;
;*** register monitor - intnr must be 21h
;*** atype:  0: protected mode
;***         1: real mode
;***         2: nohookdoscall

RegisterDosMon proc far pascal uses si di intnr:word,hookproc:dword,atype:word,mystack:word

local   ifadr:word

        mov     bx,offset iflag
        mov     cx,atype
        shl     cx,1
        add     bx,cx
        cmp     byte ptr cs:[bx+1],0
        jnz     rdm_err                   ;error: already registered
        mov     ifadr,bx

        cmp     word ptr atype,PMTYPE     ;protected mode?
        jz      rdm_1
        cmp     word ptr atype,NHTYPE     ;NoHookDosCall?
        jz      rdm_2
                                          ;real mode!
        invoke  rmreg,intnr,hookproc
        jmp     rdm_3
rdm_1:                                    ;protected mode!
        invoke  pmreg,intnr,hookproc,mystack,0
        jmp     rdm_3
rdm_2:
        invoke  pmreg,intnr,hookproc,mystack,1
rdm_3:
        and     ax,ax
        jz      rdm_ex
        mov     es,cs:[wCSalias]
        mov     bx,ifadr
        mov     byte ptr es:[bx+1],-1
        jmp     rdm_ex
rdm_err:
        xor     ax,ax
rdm_ex:
        ret
RegisterDosMon endp

;*** disable real-mode monitor

rmunreg proc near pascal intnr:word

        mov     bl,byte ptr intnr
        mov     ax,200h          ; get rm int vector
        int     31h
        mov     ax,0
        cmp     cx,cs:[codeseg]
        jnz     rur2
        cmp     dx,offset rintrou
        jnz     rur2
        mov     cx,word ptr cs:[oldintr+2]
        mov     dx,word ptr cs:[oldintr+0]
        mov     bx,intnr
        mov     ax,201h          ;set rm int vector
        int     31h
        mov     ax,1
rur2:
        ret
rmunreg endp

;*** disable protected-mode monitor
;--- if ?USEHWBREAK == 1, all that's needed is to delete the hw bp
;--- and restore exception vector 01.
;--- if ?USEHWBREAK == 0, the vectors must be restored.

pmunreg proc near pascal intnr:word,wtype:word

if ?USEHWBREAK
        mov     bx,cs:[hwhandle]
        mov     ax,0B01h
        int     31h
 if ?USEINTERRUPTREGISTER
        invoke  InterruptUnRegister,hTask
 else
        mov     bl,1
        mov     ax,202h     ; get exc vector
        int     31h
        xor     ax,ax
        mov     bx,cs
        cmp     cx,bx
        jnz     ur2
        cmp     dx,offset pintr01
        jnz     ur2

        mov     dx,word ptr cs:[oldexc01+0]
        mov     cx,word ptr cs:[oldexc01+2]
        @DPMI_SETEXCVEC 01
 endif
else
        cmp     wtype,0
        jz      @F
        invoke  GetSetKernelDosProc,cs:[oldint21p]
        jmp     exit
@@:
        mov     bl,byte ptr intnr
        mov     ax,204h
        int     31h
        mov     ax,cs
        cmp     ax,cx
        mov     ax,0                  ;kann nicht entregistriert werden
        jnz     ur2
        cmp     dx,offset pintrou
        jnz     ur2
        mov     dx,word ptr cs:[oldint21p+0]
        mov     cx,word ptr cs:[oldint21p+2]
        mov     bl,byte ptr intnr
        mov     ax,205h
        int     31h
endif
exit:
        mov     ax,1
ur2:
        ret
pmunreg endp

;*** export: unload monitor

UnregisterDosMon proc far pascal uses si di ds intnr:word,atype:word

local   ifadr:word

        xor     ax,ax
        mov     bx,offset iflag
        mov     cx,atype
        shl     cx,1
        add     bx,cx
        cmp     byte ptr cs:[bx+1],0
        jz      ur1                   ;error: nicht registriert
        mov     ifadr,bx

        cmp     word ptr atype,PMTYPE
        jz      urm_1
        cmp     word ptr atype,NHTYPE
        jz      urm_2

        invoke  rmunreg,intnr
        jmp     ur2
urm_1:
        invoke  pmunreg,intnr,0
        jmp     ur2
urm_2:
        invoke  pmunreg,intnr,1
        jmp     ur2
ur2:
        and     ax,ax
        jz      ur1
        mov     bx,ifadr
        mov     es,cs:[wCSalias]
        mov     byte ptr es:[bx+1],0
ur1:
        ret
UnregisterDosMon endp


;*** following functions were intended for Win3x.
;*** in Win9x, they're useless

AllocDosMemory proc far pascal dwBytes:dword

        invoke  GlobalDosAlloc,dwBytes
        ret
AllocDosMemory endp

FreeDosMemory proc far pascal wSel:word

        invoke  GlobalDosFree,wSel
        ret
FreeDosMemory endp

        END LibEntry

