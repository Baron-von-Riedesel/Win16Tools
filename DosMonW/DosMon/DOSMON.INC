
ZREGS struct
oax    dw ?
obx    dw ?
ocx    dw ?
odx    dw ?
osi    dw ?
odi    dw ?
obp    dw ?
osp    dw ?
ods    dw ?
oes    dw ?
oss    dw ?
ocs    dw ?
ofg    dw ?
oip    dw ?
res1   dw ?
res2   dw ?
res3   dw ?
res4   dw ?
user1  dw ?
user2  dw ?
pax    dw ?
pbx    dw ?
pcx    dw ?
pdx    dw ?
ZREGS  ends

@DPMI_SETEXCVEC macro xx
local sm1
        mov     bl,xx
        mov     ax,0203h
        int     31h
        jnc     short sm1
        mov     ax,0283h
        int     31h
sm1:
endm

@chkfn  macro
local   sm1
        cmp     al,39H                ;39 - 3D (create dir,delete dir,change dir
        jb      short sm1             ;         create file,open file)
        cmp     al,3EH
        jb      short @F
        cmp     al,41H                ;delete file
        jz      short @F
        cmp     al,43H                ;get/set file attribut
        jz      short @F
        cmp     al,4BH                ;exec program
        jz      short @F
        cmp     al,4EH                ;findfirst
        jz      short @F
        cmp     al,5BH                ;open file (extended)
        jnz     short sm1
@@:
        call    getfn
sm1:
endm

@saveall macro
        pop     [si.ZREGS.osi]
        pop     [si.ZREGS.ods]
        mov     [si.ZREGS.oax],ax
        mov     [si.ZREGS.obx],bx
        mov     [si.ZREGS.ocx],cx
        mov     [si.ZREGS.odx],dx
        mov     [si.ZREGS.odi],di
        mov     [si.ZREGS.oes],es
        mov     [si.ZREGS.oss],ss
        mov     [si.ZREGS.obp],bp
        mov     bp,sp
        lea     ax,[bp+32]
        mov     [si.ZREGS.osp],ax
        mov     ax,[bp+24]                ;dx vor call
        mov     [si.ZREGS.pdx],ax
        mov     ax,[bp+26]                ;cx vor call
        mov     [si.ZREGS.pcx],ax
        mov     ax,[bp+28]                ;bx vor call
        mov     [si.ZREGS.pbx],ax
        mov     ax,[bp+30]                ;ax vor call
        mov     [si.ZREGS.pax],ax
        mov     ax,[bp+32]                ;ip
        mov     [si.ZREGS.oip],ax
        mov     ax,[bp+34]                ;cs
        mov     [si.ZREGS.ocs],ax
        mov     ax,[bp+22]                ;flags
        mov     [si.ZREGS.ofg],ax
        endm

