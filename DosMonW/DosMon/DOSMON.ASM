
;*** DOS (int 21h) monitor - dll

	.286
	.MODEL SMALL
	.386
	option casemap:none

?USEHWBREAK equ 1
?USEINTERRUPTREGISTER equ 0 ;1=use ToolHelp InterruptRegister()

	include winproto.inc
	include dosbase.inc

	include dosmon.inc

ANZINT = 30h

PMTYPE = 0
RMTYPE = 1
NHTYPE = 2


	.CODE

pmcallback dd ?             ;not used currently

oldintp    dd ?             ;PM: addresse der vorherigen routine
phook      dd ?             ;PM: addresse der aufzurufenden routine
pmstack    dw ?             ;PM: wert des stack pointers
dsreg      dw ?             ;PM: Inhalt des DS-Registers
oldintr    dd ?             ;RM: addresse der vorherigen routine
rhook      dd ?             ;RM: addresse der tabelle zur abspeicherung der registerinhalte
rcodesel   dw ?             ;RM: code segment
sel        dw ?             ;CS alias
oldbase    dd 0

if ?USEHWBREAK
oldexc01   dd ?
oldpketvek dd ?
hwhandle   dw 0
if ?USEINTERRUPTREGISTER
hTask      dw 0
endif
endif

rflag      db 1             ;registerausgabe ein (bleibt konstant)

        align 2

regstruc   ZREGS <>
iflag      db ANZINT dup (0)

UnregisterDosMon proto far pascal intnr:word,atype:word

;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³** windows main procedure (DLL)   **³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

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

        mov     es:[sel],ax
        mov     eax,toldbase
        mov     es:[oldbase],eax

        movzx   ecx,dx             ;real mode segment in dx
        shl     ecx,4
        invoke SetSelectorBase, cs, ecx

        jmp     ind2
ind1:
        invoke  AllocCStoDSAlias,cs
        mov     es,ax
        mov     es:[sel],ax
ind2:
        mov     ax,1
        ret
LibEntry endp

;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³** windows exit procedure (DLL)   **³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
Wep     proc far pascal uses si wCode:word

        mov     bx,offset iflag
        mov     cx,ANZINT
@@:
        cmp     byte ptr cs:[bx],0
        jz      wep1
        push    bx
        push    cx

        sub     bx,offset iflag
        mov     si,bx
        invoke  UnregisterDosMon,si,0
        invoke  UnregisterDosMon,si,1
        invoke  UnregisterDosMon,si,2

        pop     cx
        pop     bx
wep1:
        inc     bx
        loop    @B

        mov     eax,cs:[oldbase]
        and     eax,eax
        jz      wep2
        push    cs:[sel]
        invoke SetSelectorBase, cs, eax
        pop     ax
        invoke  GlobalDosFree,ax
        jmp     wep3
wep2:
        invoke  FreeSelector,cs:[sel]
wep3:
        mov     ax,1
        ret
Wep     endp

;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³** filename abspeichern (RM + PM) **³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

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

;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³** register weitergeben (RM)      **³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

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

;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³** register weitergeben (PM)      **³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

pregout proc near c

        test    byte ptr cs:[rflag],01
        jz      sm111
                                       ;ip = bp + 20
        pusha                          ;bp+4 - bp + 18
        push    ds                     ;bp+2
        push    es                     ;bp+0

        push    ds
        push    si

        mov     ds,cs:[sel]
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

;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³** Interrupt 0x21 Routine RM      **³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

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

;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³** Interrupt 0x21 Routine RM + PM **³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

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
        call    dword ptr cs:[oldintp]
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
        jmp     dword ptr cs:[oldintp]
termw:                                 ;exec program
        call    pregout
        call    dword ptr cs:[oldintp]
        push    bp
        mov     bp,sp
        mov     byte ptr [bp+9],0ffh   ;ax
        pop     bp
        jmp     sm2

pintrou endp

if ?USEHWBREAK
 if ?USEINTERRUPTREGISTER

intrproc proc far
        push    bp
        mov     bp,sp

xTHIP  equ [bp+2]
xTHCS  equ [bp+4]
xAX    equ [bp+6]
xIntNr equ [bp+8]
xHdl   equ [bp+10]
xIP    equ [bp+12]
xCS    equ [bp+14]
xFL    equ [bp+16]

        cmp     word ptr xIntNr,1
        jnz     short @F
        push    eax
        mov     eax,[xIP]
        cmp     eax,cs:[oldpketvek]
        jz      short intrproc_1
        pop     eax
@@:
        pop     bp
        retf
intrproc_1:
        push    bx
        mov     bx,cs:[hwhandle]
        mov     ax,0B03h
        int     31h
        pop     bx

        mov     ax,cs
        shl     eax,16
        mov     ax,offset pintrou
        mov     [xIP],eax
        pop     eax
        mov     ax,xAX
        pop     bp
        add     sp,10
        iret
intrproc endp

 else

;*** EXC 01 routine ***
;*** wir mssen unbedingt filtern ***

pintr01 proc far
        push    bp
        mov     bp,sp

xDPMIIP equ [bp+2]
xDPMICS equ [bp+4]
xDPMIER equ [bp+6]
xIP     equ [bp+8]
xCS     equ [bp+10]
xFL     equ [bp+12]

        pushf
        push    eax
        mov     eax,xIP
        cmp     eax,cs:[oldpketvek]     ;ist es unsere Stelle?
        jz      @F
        pop     eax
        popf
        pop     bp
        jmp     dword ptr cs:[oldexc01]
@@:
        push    bx
        mov     bx,cs:[hwhandle]
        mov     ax,0B03h
        int     31h
        pop     bx                      ;reset debug watchpoint

        mov     ax,cs
        shl     eax,16
        mov     ax,offset pintrou
        mov     [xIP],eax
        pop     eax
        popf
        pop     bp
        retf
pintr01 endp

 endif

newpketvek:
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

;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³** real mode monitor registrieren **³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

rmreg  proc near pascal intnr:word,hookproc:dword

local   rmsel:word

                                           ;1.: real mode segmentaddresse -> rmsel
        push    cs
        call    GetSelectorBase
        mov     cx,ax
        xor     ax,ax                      ;rc=FALSE
        cmp     dx,000Fh
        ja      rmreg1                     ;fehler: aktuelle addresse nicht im 1. MB
        test    cx,000Fh
        jnz     rmreg1                     ;fehler: beginnt nicht an paragraphgrenze
        mov     ax,cx
        shr     ax,4
        shl     dx,12
        or      ax,dx
        mov     rmsel,ax
                                           ;2.: real mode interruptvektor holen und sichern
        mov     bl, byte ptr intnr
        mov     ax, 200h
        int     31h
        mov     es,cs:[sel]                ;ins codesegment schreiben
        assume  es:_TEXT
        mov     word ptr es:[oldintr+0],dx
        mov     word ptr es:[oldintr+2],cx

        mov     ax,word ptr hookproc+0     ;3.: die prozeduraddresse wird vorlaeufig
        mov     word ptr es:[rhook+0],ax   ;    als far pointer auf tabelle interpretiert
        mov     ax,word ptr hookproc+2
        mov     word ptr es:[rhook+2],ax
        mov     ax,rmsel
        mov     es:[rcodesel],ax

;        push    ds
;        push    es
;        mov     ax,ds
;        mov     es,ax
;        @movea  di,rmds
;        mov     si,offset introu1
;        mov     ax,cs
;        mov     ds,ax
;        @DPMI_ALLOCRMCALLBACK             ;real mode callback anfordern
;        pop     es
;        pop     ds
;        mov     word ptr es:[pmcallback+0],dx
;        mov     word ptr es:[pmcallback+2],cx
                                           ;5.: real mode vektor setzen
        mov     cx,rmsel
        mov     dx,offset rintrou
        mov     bl,byte ptr intnr
        mov     ax,201h
        int     31h
                                           ;6.: rc=TRUE
        mov     ax,1
rmreg1:
        ret
rmreg   endp


pmreg  proc near pascal intnr:word,hookproc:dword,mystack:word,wtype:word


        cmp     wtype,0
        jz      setint25
        cli
        invoke  GetSetKernelDosProc,0
        push    dx
        push    ax
        invoke  GetSetKernelDosProc,dx::ax
        sti
        pop     dx
        pop     cx
        jmp     next
setint25:
        mov     bl, byte ptr intnr
        mov     ax, 204h	;get pm int vector
        int     31h
        jc      rim_er
next:
        mov     es,cs:[sel]               ;ins codesegment schreiben
        assume  es:_TEXT
        mov     word ptr es:[oldintp+0],dx
        mov     word ptr es:[oldintp+2],cx

        mov     eax,hookproc
        mov     es:[phook],eax
        mov     ax,ds
        mov     es:[dsreg],ax
        mov     ax,mystack
        mov     es:[pmstack],ax

if ?USEHWBREAK
        push    0
        push    dx

        mov     bx,cx
        mov     ax,6	;get base
        int     31h
        push    cx
        push    dx
        pop     eax

        pop     edx
        add     eax,edx

        push    eax
        pop     cx
        pop     bx

        mov     dx,0001
        mov     ax,0B00h
        int     31h
        jc      rim_er

        mov     es,cs:[sel]
        mov     es:[hwhandle],bx
 if ?USEINTERRUPTREGISTER
        @GetCurrentTask
        mov     [hTask],ax
        invoke  InterruptRegister,ax,addr intrproc
 else
        mov     bl,1
        mov     ax,202h  ; get exception vector
        int     31h
        jc      rim_er
        mov     word ptr es:[oldexc01+0],dx
        mov     word ptr es:[oldexc01+2],cx
        mov     cx,cs
        mov     dx,offset pintr01
        @DPMI_SETEXCVEC 01
        jc      rim_er
 endif

        mov     eax,es:[oldintp]
        mov     es:[oldpketvek],eax
        mov     ax,cs
        shl     eax,16
        mov     ax,offset newpketvek
        mov     es:[oldintp],eax
else
        cmp     wtype,0
        jz      @F
        invoke  GetSetKernelDosProc,pintrou
        jmp     next2
@@:
        mov     cx,cs
        mov     dx,offset pintrou
        mov     bl,intnr
        mov     ax,205h    ; set pm int vector
        int     31h
        jc      rim_er
next2:
endif                                     ;rc=TRUE
        mov     ax,1
        ret
rim_er:
        xor     ax,ax
        ret
pmreg   endp
;
;*** export: monitor registrieren (intnr darf nur 21 sein)  ***
;*** atype:  0: protected mode ***
;***         1: real mode ***

RegisterDosMon proc far pascal uses si di intnr:word,hookproc:dword,atype:word,mystack:word

local   ifadr:word

        mov     bx,offset iflag
        add     bx,intnr
        mov     cl,byte ptr atype
        inc     cl
        or      cl,cs:[bx]
        cmp     cl,cs:[bx]
        jz      rim_er                    ;fehler: keine 2. registrierung
        mov     ifadr,bx


        cmp     word ptr atype,PMTYPE     ;protected mode?
        jz      rdm_1
        cmp     word ptr atype,NHTYPE     ;NoHookDosCall?
        jz      rdm_2
                                          ;real mode!
        invoke  rmreg,intnr,hookproc
        jmp     rim2
rdm_1:                                    ;protected mode!
        invoke  pmreg,intnr,hookproc,mystack,0
        jmp     rim2
rdm_2:
        invoke  pmreg,intnr,hookproc,mystack,1
rim2:
        and     ax,ax
        jz      rim1
        mov     es,cs:[sel]
        mov     bx,ifadr
        mov     cl,byte ptr atype
        inc     cl
        or      es:[bx],cl
        jmp     rim1
rim_er:
        xor     ax,ax
rim1:
        ret
RegisterDosMon endp

;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³** real mode monitor entregistrieren **³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
rmunreg proc near pascal intnr:word

        mov     bl,byte ptr intnr
        mov     ax,200h          ; get rm int vector
        int     31h
        mov     ax,0
        cmp     cx,cs:[rcodesel]
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

;*** protected mode monitor entregistrieren ***

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
        cmp     wType,0
        jz      @F
        invoke  GetSetKernelDosProc,cs:[oldintp]
        jmp     exit
@@:
        @DPMI_GETPMINTVEC intnr
        mov     ax,cs
        cmp     ax,cx
        mov     ax,0                  ;kann nicht entregistriert werden
        jnz     ur2
        cmp     dx,offset pintrou
        jnz     ur2
        mov     dx,word ptr cs:[oldintp+0]
        mov     cx,word ptr cs:[oldintp+2]
        @DPMI_SETPMINTVEC intnr
endif
exit:
        mov     ax,1
ur2:
        ret
pmunreg endp

;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³** export: monitor entregistrieren   **³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

UnregisterDosMon proc far pascal uses si di ds intnr:word,atype:word

local   ifadr:word

        xor     ax,ax
        mov     bx,offset iflag
        add     bx,intnr
        mov     cl,byte ptr atype
        inc     cl
        or      cl,cs:[bx]
        cmp     cl,cs:[bx]
        jnz     ur1                   ;fehler: nicht registriert
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
        mov     es,cs:[sel]
        mov     cl,byte ptr atype
        inc     cl
        xor     es:[bx],cl
ur1:
        ret
UnregisterDosMon endp


;*** folgende Funktionen waren nur vor Windows 95 sinnvoll, jetzt ***
;*** bringt es nix mehr, wenn in einem fixierten Code Segment einer ***
;*** Dll allokiert wird (frher war es automatisch im low memory) ***

AllocDosMemory proc far pascal dwBytes:dword

        invoke  GlobalDosAlloc,dwBytes
        ret
AllocDosMemory endp

FreeDosMemory proc far pascal wSel:word

        invoke  GlobalDosFree,wSel
        ret
FreeDosMemory endp

        END LibEntry

