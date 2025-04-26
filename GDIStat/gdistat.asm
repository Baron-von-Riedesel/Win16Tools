
;*** display user & gdi heap status

	.286
	.model small
	.386
	option casemap:none

HMODULE typedef WORD

	.xlist
	.xcref
	include windows.inc
	include winproto.inc
	.cref
	.list

CStr macro text:vararg
local sym
	.const
sym db text,0
	.code
	exitm <addr sym>
endm

WSTYLE	equ WS_POPUPWINDOW shl 16 ;HIWORD is important

	.data

	dw 0,0,5,0,0,0,0,0

	.const
        
szClass db "UserGDIStatus",0
szWnd   db "User & GDI Status",0

	.code

GetDataUser proc
	invoke GetModuleHandle, CStr("USER")
	invoke GetHeapSpaces, ax
	ret
GetDataUser endp

GetDataGDI proc
	invoke GetModuleHandle, CStr("GDI")
	invoke GetHeapSpaces, ax
	ret
GetDataGDI endp

GetPercent proc stdcall w1:word, w2:word
	xor ax,ax
	.if w2
		mov ax, w1
		mov cx, 100
		mul cx
		div w2
	.endif
	ret
GetPercent endp

OnTimer proc stdcall uses si di hWnd:HWND

local wGDITotal:WORD
local wGDIFree:WORD
local wUserTotal:WORD
local wUserFree:WORD
local szStr[80]:byte

	invoke GetDataUser
	mov wUserFree, ax
	mov wUserTotal, dx
ifdef _DEBUG
	invoke wsprintf, addr szStr, CStr("GetDataUser()=%lX",13,10), dx::ax
	invoke OutputDebugString, addr szStr
endif

	invoke GetDataGDI
	mov wGDIFree, ax
	mov wGDITotal, dx
ifdef _DEBUG
	invoke wsprintf, addr szStr, CStr("GetDataGDI()=%lX",13,10), dx::ax
	invoke OutputDebugString, addr szStr
endif

	invoke InvalidateRect, hWnd, 0, 1
	invoke GetPercent, wUserFree, wUserTotal
	mov si,ax
	invoke GetPercent, wGDIFree, wGDITotal
	mov di,ax
	invoke wsprintf, addr szStr, CStr("U/G:%u%%/%u%%"), si, di
	invoke SetWindowText, hWnd, addr szStr
	ret
OnTimer endp

PaintIconic proc stdcall hWnd:HWND, hDC:HDC

local rect:RECT
local hBrush:HBRUSH
local wRight:WORD

	invoke GetClientRect, hWnd, addr rect
	mov ax, rect.rcRight
	mov wRight,ax
	shr ax, 1
	sub ax, 1
	mov rect.rcRight, ax

	invoke GetDataUser
	invoke GetPercent, ax, dx
	call paintrect

	mov ax,rect.rcRight
	add ax,1+1
	mov rect.rcLeft,ax
	mov ax,wRight
	mov rect.rcRight,ax
	invoke GetDataGDI
	invoke GetPercent, ax, dx
	call paintrect
	ret

paintrect:
	.if ax > 60
		mov ecx, 00c000h
	.elseif ax > 20
		mov ecx, 000c0c0h
	.else
		mov ecx, 0c0h
	.endif
	push ax
	invoke CreateSolidBrush, ecx
	mov hBrush, ax
	pop dx
	mov ax,rect.rcBottom
	mul dx
	mov bx,100
	div bx
	mov dx,rect.rcBottom
	sub dx,ax
	mov rect.rcTop, dx
	invoke FillRect, hDC, addr rect, hBrush
	invoke DeleteObject, hBrush
	retn
PaintIconic endp

OnPaint proc stdcall hWnd:HWND, wParam:WPARAM, lParam:LPARAM

local  ps:PAINTSTRUCT

	invoke IsIconic, hWnd
	.if ax
		invoke BeginPaint, hWnd, addr ps
		invoke PaintIconic, hWnd, ps.hdc
		invoke EndPaint, hWnd, addr ps
	.else
		invoke DefWindowProc, hWnd, WM_PAINT, wParam, lParam
	.endif
	ret
OnPaint endp

;*** window procedure

WndProc proc far pascal hWnd:HWND,message:UINT,wParam:WPARAM,lParam:LPARAM

	mov ax,message
	.if ax == WM_CREATE
		invoke SetTimer, hWnd, 1, 2000, 0
;	.elseif ax == WM_COMMAND
;		invoke OnCommand, hWnd, wParam, lParam
	.elseif ax == WM_DESTROY
		invoke KillTimer, hWnd, 1
		invoke PostQuitMessage, 0
		xor ax,ax
		cwd
	.elseif ax == WM_PAINT
		invoke OnPaint, hWnd, wParam, lParam
	.elseif ax == WM_TIMER
		invoke OnTimer, hWnd
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

	mov wc.hIcon,0

	invoke LoadCursor, 0, IDC_ARROW
	mov wc.hCursor,ax

	mov ax,COLOR_BACKGROUND + 1
	mov wc.hbrBackground,ax

	xor eax,eax
	mov wc.lpszMenuName,eax

	mov word ptr wc.lpszClassName+0,offset szClass
	mov word ptr wc.lpszClassName+2,ds

	invoke RegisterClass, addr wc
exit:
	ret
InitApplication endp

;*** create main window and show it

InitInstance  proc stdcall hInstance:HINSTANCE

;local   hMenu:HMENU

;	invoke LoadMenu, hInstance,IDR_MENU1
;	mov   hMenu,ax

	invoke CreateWindow, addr szClass,addr szWnd,WSTYLE,CW_USEDEFAULT,CW_USEDEFAULT,\
		 CW_USEDEFAULT,CW_USEDEFAULT,0,NULL,hInstance,0
	.if (ax)
		push ax
		invoke ShowWindow, ax, SW_SHOWMINIMIZED
		pop ax
	.endif
exit:
	ret
InitInstance  endp

;*** WINMAIN ***

WinMain proc pascal hInstance:HINSTANCE,hPrevInstance:HINSTANCE,lpszCmdline:LPSTR,cmdshow:UINT

;local   hAccelTable:WORD
local   msg:MSGSTRUCT

	.if hPrevInstance == 0				;1. Instance?
		invoke InitApplication,hInstance
		and ax,ax
		jz exit
	.endif

	invoke InitInstance,hInstance
	and ax,ax
	jz exit
	.while 1
		invoke GetMessage, addr msg,0,0,0
		.break .if (ax == 0)
		invoke TranslateMessage, addr msg
		invoke DispatchMessage, addr msg
	.endw
exit:
	ret
WinMain endp

start:
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

	end start
