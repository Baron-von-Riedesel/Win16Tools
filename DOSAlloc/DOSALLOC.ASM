
;*** test DOS/DPMI memory alloc

	.286
	.model small
	option casemap:none
	.386

FILLMEM equ 1

	.nolist
	.nocref
	include windows.inc
	include winproto.inc
	include dpmi.inc
	.cref
	.list

	include dosalloc.inc

WSTYLE  equ ((WS_OVERLAPPEDWINDOW) shl 16)  ;HIWORD is important!
EDITSTYLE equ ((WS_CHILD or WS_VISIBLE or WS_VSCROLL) shl 16) or (ES_READONLY or ES_MULTILINE)
ID_EDIT equ 1

CStr macro text:vararg
local sym
	.const
sym db text,0
	.code
	exitm <addr sym>
endm

BLKITEM struct
pNext dw ?
dwHdl dd ?
BLKITEM ends

	.data
	dw 0,0,5,0,0,0,0,0

hwndMain dw 0
hwndEdit dw 0
hAccel   dw 0
meminfo MEMINFO <>
pBlocks  dw 0
if FILLMEM
wFlat    dw 0
endif

	.const

szClass     db "DOS/DPMI memory alloc test",0
szWnd       db "DOS/DPMI memory alloc test",0
szNC        db "NC",0
szC         db "C",0
szNull      db 0

	.code

;--- add a line to the edit control

AddLine proc c pszText:word

local szStr[80]:byte

	mov ax,pszText
	invoke wsprintf, addr szStr, CStr("%s",13,10), ss::ax
	invoke SendMessage, hwndEdit, WM_GETTEXTLENGTH, 0, 0
	invoke SendMessage, hwndEdit, EM_SETSEL, 0, ax::ax
	invoke SendMessage, hwndEdit, EM_REPLACESEL, 0, addr szStr
;	invoke OutputDebugString, addr szStr
	ret
AddLine endp

;--- display DPMI memory status

status proc c uses si di

local szStr[80]:byte

	mov di,offset meminfo
	push ds
	pop es
	mov ax,500h              ; DPMI alloc - won't be released if task terminates!
	int 31h
	mov eax,[di].MEMINFO.freeUnlocked
	mov ecx,[di].MEMINFO.freePhys
	mov edx,[di].MEMINFO.freeAdrSpace
	.if [di].MEMINFO.totalPhys > (64 shl 8)
		;convert 4k to 1M
		shr eax,8
		shr ecx,8
		shr edx,8
		invoke wsprintf, addr szStr, CStr("virt free: %lu MB, phys free: %lu MB, space free: %lu MB"),eax,ecx,edx
	.else
		;convert 4k to 1k
		shl eax,2
		shl ecx,2
		shl edx,2
		invoke wsprintf, addr szStr, CStr("virt free: %lu kB, phys free: %lu kB, space free: %lu kB"),eax,ecx,edx
	.endif
	invoke AddLine, addr szStr
	ret
status endp

;--- alloc DPMI memory
;--- store handle in a linked list in local heap, so the memory can be released later.

dpmi_alloc proc c uses si di dwSize:dword

local dwAddr:dword
local bFlags:byte
local szStr[128]:byte

	mov cx,word ptr dwSize+0
	mov bx,word ptr dwSize+2
	mov ax,501h
	int 31h
	lahf
	mov bFlags,ah
	mov word ptr dwAddr+0,cx
	mov word ptr dwAddr+2,bx
	mov dx,offset szC
	jc @F

;--- store handle in a linked local heap item

	invoke LocalAlloc, LMEM_FIXED, sizeof BLKITEM
	.if ax
		mov bx,ax
		mov word ptr [bx].BLKITEM.dwHdl+0,di
		mov word ptr [bx].BLKITEM.dwHdl+2,si
		xchg ax, pBlocks
		mov [bx].BLKITEM.pNext, ax
	.endif

;--- display result of allocation in edit control

	mov dx,offset szNC
@@:
	invoke wsprintf, addr szStr, CStr("int 31h (ax=501h bx:cx=%lX) returned %s, bx:cx=%lX"), dwSize, ds::dx, dwAddr
	invoke AddLine, addr szStr

if FILLMEM
	test bFlags,1
	jnz @F
	mov ax,wFlat
	and ax,ax
	jz @F
	mov es,ax
	mov edi,dwAddr
	cld
	mov eax,0DEADBEEFh
	mov ecx,dwSize
	shr ecx,2
	rep stosd [edi]
@@:
endif
	ret
dpmi_alloc endp

;--- free DPMI memory

dpmi_dealloc proc uses si di

local szStr[128]:byte

	xor bx,bx
	xchg bx,pBlocks
	.while bx
		mov di, word ptr [bx].BLKITEM.dwHdl+0
		mov si, word ptr [bx].BLKITEM.dwHdl+2
		mov ax,502h
		int 31h
		mov dx,offset szC
		jc @F
		mov dx,offset szNC
@@:
		push bx
		invoke wsprintf, addr szStr, CStr("int 31h (ax=502h, si:di=%lX) returned %s"), si::di, ds::dx
		invoke AddLine, addr szStr
		pop bx
		push [bx].BLKITEM.pNext
		invoke LocalFree, bx
		pop bx
	.endw
	ret
dpmi_dealloc endp

;--- alloc memory with DOS int 21h, ah=48h

dos_alloc proc

local szStr[128]:byte

	mov ah,48h               ; DOS alloc - won't be released if task terminates!
	mov bx,0FFFEh
	int 21h
	mov dx,offset szC
	jc @F
	mov dx,offset szNC
@@:
	invoke wsprintf, addr szStr, CStr("int 21h (ah=48h, BX=FFFEh) returned %s, ax=%X"), ss::dx, ax
	invoke AddLine, addr szStr
	ret
dos_alloc endp

;*** WM_COMMAND handler

OnCommand proc stdcall hWnd:HWND,wParam:WPARAM,lParam:LPARAM

	mov ax,wParam
	.if (ax == IDM_EXIT)
		invoke PostMessage, hWnd,WM_CLOSE,0,0
	.elseif (ax == IDM_ALLOC1)
		invoke dos_alloc
	.elseif ax == IDM_ALLOC2 || ax == IDM_ALLOC3 || ax == IDM_ALLOC4
		.if ax == IDM_ALLOC2
			mov bx,10h
		.elseif ax == IDM_ALLOC3
			mov bx,100h
		.else
			mov bx,1000h
		.endif
		mov cx,0
		invoke dpmi_alloc, bx::cx
	.elseif ax == IDM_DEALLOC
		invoke dpmi_dealloc
	.elseif ax == IDM_STATUS
		invoke status
	.else
		xor ax,ax
		cwd
	.endif
	ret

OnCommand endp

;*** window proc

WndProc proc far pascal hWnd:HWND,message:UINT,wParam:WPARAM,lParam:LPARAM

local rect:RECT

	mov ax,message
	.if (ax == WM_COMMAND)
		invoke OnCommand,hWnd,wParam,lParam
	.elseif (ax == WM_SIZE)
		invoke GetClientRect, hWnd, addr rect
		invoke SetWindowPos, hwndEdit, NULL, 0, 0, rect.rcRight, rect.rcBottom, SWP_NOZORDER or SWP_NOMOVE or SWP_NOACTIVATE
	.elseif (ax == WM_DESTROY)
		invoke PostQuitMessage, 0
		xor ax,ax
		cwd
	.else
		invoke DefWindowProc, hWnd,message,wParam,lParam
	.endif
	ret
WndProc endp

;*** InitApplication registers window class

InitApplication proc stdcall hInstance:HINSTANCE

local   wc:WNDCLASS

	mov wc.style,0
	mov word ptr wc.lpfnWndProc+0,offset WndProc
	mov word ptr wc.lpfnWndProc+2,cs
	xor ax,ax
	mov wc.cbClsExtra,ax
	mov wc.cbWndExtra,ax

	mov ax,hInstance
	mov wc.hInstance,ax

	invoke LoadIcon, hInstance,IDI_ICON1
	mov wc.hIcon,ax

	invoke LoadCursor, 0,IDC_ARROW
	mov wc.hCursor,ax

	mov ax,COLOR_WINDOW + 1
	mov wc.hbrBackground,ax

	xor eax,eax
	mov wc.lpszMenuName,eax

	mov word ptr wc.lpszClassName+0,offset szClass
	mov word ptr wc.lpszClassName+2,ds

	invoke RegisterClass, addr wc
exit:
	ret
InitApplication endp

;*** InitInstance creates main window and shows it 

InitInstance  proc stdcall hInstance:HINSTANCE

local   hWnd:HWND
local   hMenu:HMENU
local   rect:RECT

	invoke LoadMenu, hInstance,IDR_MENU1
	mov   hMenu,ax

	invoke CreateWindow, addr szClass,addr szWnd,WSTYLE,\
		CW_USEDEFAULT,CW_USEDEFAULT,512,256,0,hMenu,hInstance,0
	.if (ax)
		mov hWnd,ax
		mov hwndMain,ax
		invoke GetClientRect, hWnd, addr rect
		invoke CreateWindow, CStr("edit"),addr szNull,EDITSTYLE,
			rect.rcLeft,rect.rcTop,rect.rcRight,rect.rcBottom,hWnd,ID_EDIT,hInstance,0
		mov hwndEdit,ax
		invoke ShowWindow, hWnd,SW_SHOWNORMAL
if FILLMEM
		invoke AllocSelector,0
		mov wFlat,ax
 if 0
		invoke SetSelectorBase,ax,0
;--- might set limit to 0fffffh only!
		invoke SetSelectorLimit,wFlat,-1
 else
		mov bx,wFlat
		xor dx,dx
		xor cx,cx
		mov ax,7
		int 31h
		mov cx,ds
		lar cx,cx
		shr cx,8
		or ch,80h
		mov ax,9
		int 31h
		mov dx,-1
		mov cx,-1
		mov ax,8
		int 31h
 endif
		sub sp,128
		mov dx,sp
		mov cx,wFlat
		lsl eax,ecx
		invoke wsprintf, ss::dx, CStr("flat selector: %X, limit=%lX"), wFlat, eax
		invoke AddLine, sp
		add sp,128        
endif
		mov ax,hWnd
	.endif
	ret
InitInstance  endp

;*** WINMAIN ***

WinMain proc pascal hInstance:HINSTANCE,hPrevInstance:HINSTANCE,lpszCmdline:LPSTR,cmdshow:UINT

local   hAccelTable:WORD
local   msg:MSGSTRUCT


	.if hPrevInstance == 0				;1. Instance?
		invoke InitApplication,hInstance
		and  ax,ax
		jz exit
	.endif

	invoke InitInstance,hInstance
	and  ax,ax
	jz exit

	invoke LoadAccelerators, hInstance, IDA_ACCEL1
	mov hAccel, ax

	.while 1
		invoke GetMessage, addr msg,0,0,0
		.break .if (ax == 0)
		invoke TranslateAccelerator, hwndMain, hAccel, addr msg
		.continue .if ax
		invoke TranslateMessage, addr msg
		invoke DispatchMessage, addr msg
	.endw

if FILLMEM
;--- calling FreeSelector with unmodified selector will cause a crash
	mov dx,-1
	xor cx,cx       ;set limit to 64 kB
	mov bx,wFlat
	mov ax,8
	int 31h
	invoke FreeSelector, bx
endif

exit:
	ret
WinMain endp

ifdef ?NOCRT
;--- dont use any external startup code
_start:
	invoke InitTask
	and ax,ax
	jz error
	push es
	pusha
	invoke WaitEvent,0
	invoke InitApp,di
	popa
	pop es
	invoke WinMain, di, si, es::bx, dx

	mov ah,4ch
	int 21h
error:
	mov ax,4c01h
	int 21h
startlabel equ <_start>
else
startlabel equ <>
endif

	end startlabel
