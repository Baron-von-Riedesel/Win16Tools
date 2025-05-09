        pushcontext listing
        .xlist
        .xcref

;*** definitions DOS Protected Mode Interface ***

RMCS    struct          ;real mode call structure
union
rEDI    dd ?            ;+0
rDI		dw ?
ends
union
rESI    dd ?            ;+4
rSI		dw ?
ends
union
rEBP    dd ?            ;+8
rBP		dw ?
ends
RESERVE dd ?            ;+12
union
rEBX    dd ?            ;+16
rBX     dw ?
ends
union
rEDX    dd ?            ;+20
rDX     dw ?
ends
union
rECX    dd ?            ;+24
rCX		dw ?
ends
union
rEAX    dd ?            ;+28
rAX		dw ?
ends
rFlags  dw ?            ;+32
rES     dw ?            ;+34
rDS     dw ?            ;+36
rFS     dw ?            ;+38    
rGS     dw ?            ;+40  
union
rCSIP	dd ?            ;+42
struct
rIP     dw ?
rCS     dw ?
ends
ends
union
rSSSP	dd ?            ;+46
struct
rSP     dw ?
rSS     dw ?
ends
ends
RMCS    ends

MEMINFO struct
maxBlock      dd ?            ;0	;in bytes
freeUnlocked  dd ?            ;4	;pages (max unlocked allocation)
maxLockable   dd ?            ;8	;pages (max locked allocation)
totalAdrSpace dd ?            ;12
unlocked      dd ?            ;16
freePhys      dd ?            ;20
totalPhys     dd ?            ;24
freeAdrSpace  dd ?            ;28
swapFile      dd ?            ;32
res           db 12 dup (?)
MEMINFO ends

EXCFRAME struct	;DPMI v0.9 32-bit exception frame
_dpmieip	dd ?
_dpmics		dd ?
errorcode	dd ?
_eip		dd ?
_cs			dd ?
_eflags		dd ?
_esp		dd ?
_ss			dd ?
EXCFRAME ends

EXCFR16 struct	;DPMI v0.9 16-bit exception frame
_dpmiip		dw ?
_dpmics		dw ?
errorcode	dw ?
_ip			dw ?
_cs			dw ?
_flags		dw ?
_sp			dw ?
_ss			dw ?
EXCFR16 ends

EXCFRAME10 struct	;DPMI v1.0 32-bit exception frame
	EXCFRAME <>
rDPMIIPx	dd ?	;+20h
rDPMICSx	dd ?	;+24h
rErrx		dd ?	;+28h	;filled with dr6 if exc 01
rEIPx		dd ?	;+2Ch
rCSx		dw ?	;+30h
rInfoBits	dw ?	;+32h
rEFLx		dd ?	;+34h
rESPx		dd ?	;+38h
rSSx		dd ?	;+3Ch
rESx		dd ?	;+40h
rDSx		dd ?	;+44h
rFSx		dd ?	;+48h
rGSx		dd ?	;+4Ch
rCR2		dd ?	;+50h	;exc 0E only
rPTE		dd ?	;+54h	;exc 0E only, bits 0-8 of PTE only
EXCFRAME10 ends


DPMI_AllocDesc    equ 0
DPMI_FreeDesc     equ 1
DPMI_GetBase      equ 6
DPMI_SetBase      equ 7
DPMI_SetLimit     equ 8
DPMI_SetAccRights equ 9

        popcontext listing
