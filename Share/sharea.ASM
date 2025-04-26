
	.286
	.model small
	.stack
	option casemap:none

InitTask proto far pascal
WaitEvent proto far pascal :word
GetModuleHandle proto far pascal :dword
LoadLibrary proto far pascal :dword
FreeLibrary proto far pascal :word

InitApp proto far pascal :word
MessageBox proto far pascal :word, :dword, :dword, :word

	.data
	db 16 dup (0)

	.code
szShare db ".\share.dll",0
szNotLoaded db "share.dll not loaded",0
szLoaded db "share.dll loaded",0
szUnloaded db "share.dll unloaded",0
szError db "Error",0
szInfo  db "Info",0
start:
	invoke InitTask
	and ax,ax
	jz error
	invoke WaitEvent,0
	invoke InitApp,di
	invoke GetModuleHandle, addr szShare
	.if ax
		invoke FreeLibrary, ax
		mov si,offset szUnloaded
		mov di,offset szInfo
	.else
		invoke LoadLibrary, addr szShare
		.if ax >= 32
			mov si, offset szLoaded
			mov di, offset szInfo
		.else
			mov si, offset szNotLoaded
			mov di, offset szError
		.endif
	.endif
	invoke MessageBox, 0, cs::si, cs::di, 0
	mov ax,4c00h
	int 21h
error:
	mov ax,4c01h
	int 21h

	END start

