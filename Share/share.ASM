
;--- Win16 version of fakeshare

	.286
	.model small
	option casemap:none

?USEGETSETDOSPROC equ 1

IRETS struct
wIP dw ?
wCS dw ?
wFL dw ?
IRETS ends

AllocCStoDSAlias proto far pascal :word
FreeSelector     proto far pascal :word
GetSetKernelDosProc proto far pascal :far ptr

	.code

myint21 proc
;	cmp ax,5c00h
	cmp ah,5ch
	jz is5c
	cmp ah,3Dh
	jz is3d
@@:
	db 0eah
dwOldVec dd 0
is3d:
	and al,8fh
	jmp @B
is5c:
	push bp
	mov bp,sp
	and [bp+2].IRETS.wFL,not 1
	pop bp
	iret
myint21 endp

LibMain proc far pascal uses ds

	mov bx,cs
	invoke AllocCStoDSAlias, bx
	and ax,ax
	jz exit
	mov ds,ax
	assume ds:_TEXT
if ?USEGETSETDOSPROC
	cli
	invoke GetSetKernelDosProc,addr myint21
	mov word ptr [dwOldVec+0],ax
	mov word ptr [dwOldVec+2],dx
	sti
else
	mov ax,204h
	mov bl,21h
	int 31h
	mov word ptr [dwOldVec+0],dx
	mov word ptr [dwOldVec+2],cx
	mov cx,cs
	mov dx,offset myint21
	mov ax,205h
	int 31h
endif
	mov bx,ds
	push 0
	pop ds
	invoke FreeSelector, bx
	mov ax,1
exit:
	ret
LibMain endp

	assume ds:DGROUP

Wep proc far pascal wCode:word

if ?USEGETSETDOSPROC
	invoke GetSetKernelDosProc,[dwOldVec]
else
	mov dx,word ptr [dwOldVec+0]
	mov cx,word ptr [dwOldVec+2]
	mov bl,21h
	mov ax,205h
	int 31h
endif
	mov ax,1
	ret
Wep endp

	END LibMain

