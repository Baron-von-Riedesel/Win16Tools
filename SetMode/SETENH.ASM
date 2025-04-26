
;*** set enhanced/standard mode in Win31

	.286
	.MODEL SMALL
	option casemap:none

?LOG equ 0

WFOFS equ 324h	;offset winflags in kernel dgroup

CStr macro text:vararg
local sym
	.const
sym db text,0
	.code
	exitm <addr sym>
endm

SENTRY struct
bType  db ?
bFlags db ?
bSegNo db ?
wOfs   dw ?
SENTRY ends

ENTRYSTR struct
wFirst dw ?
wLast  dw ?
wNext  dw ?
ENTRYSTR ends

ENTRY struct
bFlags db ?
wSegNo dw ?
wOfs   dw ?
wID    dw ?
lpSE   dd ?	;SENTRY far ptr
lpES   dd ?	;ENTRYSTR far ptr
ENTRY ends

InitTask        proto far pascal
WaitEvent       proto far pascal :word
InitApp         proto far pascal :word
GetModuleHandle proto far pascal :ptr far
GetWinFlags     proto far pascal
if ?LOG
OutputDebugString proto far pascal :ptr far
wsprintf        proto far c      :far ptr, :far ptr, :vararg
endif
MessageBox      proto far pascal :word, :far ptr, :far ptr , :word

;externdef pascal _WINFLAGS:far
;GETWINFLAGS_NO equ 132
_WINFLAGS_NO equ 178

GetFirstEntry   proto far pascal :word, :far ptr ENTRY
GetNextEntry    proto far pascal :word, :far ptr ENTRY
GetModuleDgroup proto far pascal :word

	.data
	db 16 dup (0)

	.const
szInfo db "Info",0

	.code

main proc c

local dgrp:word
local winflags:word
local entry:ENTRY
local szStr[80]:byte

	invoke GetModuleHandle, CStr("KERNEL")
	and ax,ax
	jz error1
	mov si,ax
	invoke GetModuleDgroup, si
	and ax,ax
	jz error2
	mov dgrp,ax
	invoke GetWinFlags
	mov winflags,ax
	invoke GetFirstEntry, si, addr entry
	.while ax
		.if entry.wID == _WINFLAGS_NO
if ?LOG
			les di, entry.lpSE
			invoke wsprintf, addr szStr, CStr("Entry %u: %X:%X at %X:%X",10), entry.wID, entry.wSegNo, entry.wOfs, es, di
			invoke OutputDebugString, addr szStr
endif
			les di, entry.lpSE
			mov ax,winflags
			.if (ax == es:[di].SENTRY.wOfs)
				xor es:[di].SENTRY.wOfs, 0030h	;switch enhanced <-> standard
				mov es,dgrp
				mov di,WFOFS
				.if (ax == word ptr es:[di])
					xor word ptr es:[di], 0030h	;switch enhanced <-> standard
				.endif
				.if ax & 10h
					invoke MessageBox, 0, CStr("Switched to Enhanced Mode"), addr szInfo, 0
				.else
					invoke MessageBox, 0, CStr("Switched to Standard Mode"), addr szInfo, 0
				.endif
			.endif
			ret
		.endif
		invoke GetNextEntry, si, addr entry
	.endw
	invoke MessageBox, 0, CStr("_WINFLAGS entry not found"), 0, 0
	ret
error1:
	invoke MessageBox, 0, CStr("GetModuleHandle('KERNEL') failed"), 0, 0
	ret
error2:
	invoke MessageBox, 0, CStr("DGROUP for KERNEL not found"), 0, 0
	ret
main endp

start:
	invoke InitTask
	invoke WaitEvent,0
	invoke InitApp,di
	call main
	mov ah,4ch
	int 21h

	end start

