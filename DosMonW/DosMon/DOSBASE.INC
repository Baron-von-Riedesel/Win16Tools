        pushcontext listing
        .xlist
        .xcref

;*** konstanten und macros MSDOS ***

;*** konstanten MSDOS ***

@DOS    equ     21h

;*** DOS Funktionen aus msdos.inc ***

DOS_kill        equ     00H             ; terminate
DOS_echoread    equ     01H             ; read keyboard and echo
DOS_display     equ     02H             ; display character
DOS_auxinput    equ     03H             ; auxiliary input
DOS_auxoutput   equ     04H             ; auxiliary output
DOS_print       equ     05H             ; print character
DOS_conio       equ     06H             ; direct console i/o
DOS_coninput    equ     07H             ; direct console input
DOS_readkbd     equ     08H             ; read keyboard
DOS_message     equ     09H             ; display string
DOS_bufkbdin    equ     0aH             ; buffered keyboard input
DOS_kbdstatus   equ     0bH             ; check keyboard status
DOS_flshread    equ     0cH             ; flush buffer and read keyboard
DOS_diskreset   equ     0dH             ; disk reset
DOS_selectdisk  equ     0eH             ; select default disk
DOS_fcbopen     equ     0fH             ; open file with fcb
DOS_fcbclose    equ     10H             ; close file with fcb
DOS_fcbfirst    equ     11H             ; search for first entry with fcb
DOS_fcbnext     equ     12H             ; search for next entry with fcb
DOS_fcbdelete   equ     13H             ; delete file with fcb
DOS_fcbsread    equ     14H             ; sequential read with fcb
DOS_fcbswrite   equ     15H             ; sequential write with fcb
DOS_fcbcreate   equ     16H             ; create file with fcb
DOS_fcbrename   equ     17H             ; rename file with fcb
DOS_currentd    equ     19H             ; current default disk
DOS_setDMA      equ     1aH             ; set DMA
DOS_fcbrread    equ     21H             ; random read with fcb
DOS_fcbrwrite   equ     22H             ; random write with fcb
DOS_fcbsize     equ     23H             ; file size with fcb
DOS_fcbsetrec   equ     24H             ; set relative record with fcb
DOS_setvector   equ     25H             ; set interrupt vector
DOS_fcbbread    equ     27H             ; random block read with fcb
DOS_fcbbwrite   equ     28H             ; random block write with fcb
DOS_fcbparse    equ     29H             ; parse file name with fcb
DOS_getdate     equ     2aH             ; get date
DOS_setdate     equ     2bH             ; set date
DOS_gettime     equ     2cH             ; get time
DOS_settime     equ     2dH             ; set time
DOS_verify      equ     2eH             ; set/reset verify flag
DOS_getDMA      equ     2fH             ; get DMA
DOS_version     equ     30H             ; get version number
DOS_keep        equ     31H             ; keep process
DOS_cntlc       equ     33H             ; Cntl-C check
DOS_getvector   equ     35H             ; get interrupt vector
DOS_getdskspc   equ     36H             ; get disk free space
DOS_country     equ     38H             ; get country dependent info
DOS_mkdir       equ     39H             ; make subdirectory
DOS_rmdir       equ     3aH             ; remove subdirectory
DOS_chdir       equ     3bH             ; change subdirectory
DOS_create      equ     3cH             ; create pathname
DOS_open        equ     3dH             ; open pathname
DOS_close       equ     3eH             ; close file handle
DOS_read        equ     3fH             ; read from file handle
DOS_write       equ     40H             ; write from file handle
DOS_delete      equ     41H             ; delete pathname
DOS_lseek       equ     42H             ; move file pointer
DOS_filemode    equ     43H             ; get/set attributes of pathname
DOS_ioctl       equ     44H             ; ioctl for devices
DOS_dup         equ     45H             ; duplicate file handle
DOS_forcedup    equ     46H             ; force duplicate file handle
DOS_curdir      equ     47H             ; get current directory
DOS_allocmem    equ     48H             ; allocate memory block
DOS_freemem     equ     49H             ; free memory block
DOS_setmem      equ     4aH             ; set size of memory block
DOS_exec        equ     4bH             ; load and execute program
DOS_terminate   equ     4cH             ; terminate process with errorcode
DOS_wait        equ     4dH             ; get child process return code
DOS_findfirst   equ     4eH             ; find first file match
DOS_findnext    equ     4fH             ; find next file match
DOS_setpsp      equ     50h             ; (undok) set psp
DOS_listoflists equ     52h             ; (undok) get list of lists
DOS_getverify   equ     54H             ; return current verify flag
DOS_rename      equ     56H             ; rename pathname
DOS_filedate    equ     57H             ; get/set file handle date/time
DOS_createnew   equ     5BH             ; create new file
DOS_locking     equ     5CH             ; file record locking(AL=0)/unlocking(AL=1)
DOS_getmachname equ     5E00H           ; get machine name (DOS 3.1)
DOS_setprninit  equ     5E02H           ; Drucker Initialisierungsstring setzen (DOS 3.1)
DOS_getprninit  equ     5E03H           ; Drucker Initialisierungsstring lesen (DOS 3.1)
DOS_getnetname  equ     5F02H           ; Eintrag aus Zuweisungsliste holen (DOS 3.1)
DOS_setnetname  equ     5F03H           ; Eintrag in Zuweisungsliste einfÅgen (DOS 3.1)
DOS_delnetname  equ     5F04H           ; Eintrag in Zuweisungsliste lîschen (DOS 3.1)
DOS_getpspseg   equ     62H             ; PSP Segmentaddresse ermitteln (DOS 3.0)
DOS_getcodpag   equ     6601H           ; Code Page lesen (DOS 3.3)
DOS_setgblcp    equ     6602H           ; Set Global Code Page (DOS 3.3)
DOS_sethdlmax   equ     67H             ; Set Handle Limit (DOS 3.3)
DOS_sleep       equ     89H             ; delay process execution

;*** macros MSDOS ***

@DosCall        macro xx
        ifnb <xx>
           mov ah,xx
        endif
        int    @DOS
        endm

@DosExit        macro xx
        ifnb <xx>
           mov     al,xx
        endif
        @DosCall DOS_terminate
        endm

;*** DOS (far) memory allokieren ***

@DOSfmalloc macro xx
        mov bx,xx               ;anzahl paragraphs!
        @DosCall DOS_allocmem
        endm

@DosGetTime macro timestruct
        @DosCall DOS_gettime
if ?use16
        @movea bx,timestruct
        xchg ch,cl
        mov [bx+0],cx
        xchg dh,dl
        mov [bx+2],dx
else
        mov  ebx,timestruct
        xchg ch,cl
        mov [ebx+0],cx
        xchg dh,dl
        mov [ebx+2],dx
endif
        endm

;--- LFN finddata structure

ifndef MAXPATH
MAXPATH equ 260
endif

FINDDATA struct
dwFileAttributes dd ?
ftCreationTime	 dq ?
ftLastAccessTime dq ?
ftLastWriteTime  dq ?
nFileSizeHigh	 dd ?
nFileSizeLow	 dd ?
dwReserved0 	 dd ?
dwReserved1 	 dd ?
cFileName		 db MAXPATH dup (?)
cAlternateFileName db 14 dup (?)
FINDDATA ends

FILEENTRY STRUCT
    feAddr            DD ?		;+0
    feAnzahl          DW ?		;+4
    feIndex           DW ?      ;+6 index in der filetabelle (00 - FF) */
    feLength          DW ?      ;+8 laenge eines eintrags (DOS abhaengig) */
    feOffset          DW ?      ;+10 aktueller offset
    feMode            DW ?		;+12
    feHandles         DW ?      ;+14 anzahl handles, 0 = file nicht geoeffnet */
    feOpen_mode       DW ?      ;+16
    feDevinfo         DW ?      ;+18 device info */
    feW1              DW 13 dup (?)	;+20
    feName            DB 8 dup (?)	;+46
    feExt             DB 3 dup (?)	;+54
    feShare_pref_sft  DD ?			;+57
    feShare_net_machine DW ?		;+61
    fePSP             DW ?			;+63
    feW2              DW 10 dup (?)	;+65 (size 85)
FILEENTRY ENDS

MCBENTRY STRUCT
    meAddr DD ?                  ;* 0: real mode addresse */
    meSize DD ?                  ;* 4 */
    meName DB 9 DUP (?)          ;* 8 */
    mePSP  DW ?                  ;* 17 */
    meNAddr DD ?                 ;* 19 */
    meSel  DW ?                  ;* 23: selector (nur temporaer gueltig) */
    meMode DB ?                  ;* 25: 1 = protected mode */
    meKB   DB ?                  ;* 26: Kennbyte
    meSK   DW ?                  ;* 27: Systemkennung, falls Name=SYSTEM
MCBENTRY ENDS


CDSENTRY STRUCT
   ceLastdrive DB ?
   ceDrive     DB ?
   ceEntry     DD ?
   ceDistance  DW ?
   ceRoot      DB ?
   ceFrei      DB ?
   ceFlags     DW ?
   ceDPB       DD ?
   cePath      DB 68 DUP (?);
CDSENTRY ENDS


@savesda macro area
        push    ds
        mov     ax,5D06h
        @DosCall
        pop     es
        jc      @F
        @movea  di,area
        push    cx
        rep     movsb
        pop     cx
@@:
        mov     ax,es
        mov     ds,ax
        endm

@ressda   macro area

        push    ds
        mov     ax,5D06h
        @DosCall
        mov     ax,ds
        mov     es,ax
        pop     ds
        jc      @F
        mov     di,si
        @movea  si,area
        rep     movsb
@@:
        endm

__STDOUT equ 1
__STDERR equ 2

;*** routine in interrupt einklinken ***

@ChangeIntVector macro nr,routine,altvektor
        ifnb <altvektor>
          if ((.type nr) and 0004h)       ;konstante oder not defined?
              mov ax,(3500h or nr)
          else
              mov al,nr
              mov ah,35h
          endif
          push    es
          push    ax
          @DosCall
          pop     ax
          mov     cx,es
          pop     es
          mov     word ptr altvektor+0,bx
          mov     word ptr altvektor+2,cx
        else
          mov al,nr
        endif
        push    ds
        mov     dx,cs
        mov     ds,dx
        mov     dx,offset routine
        mov     ah,25h
        @DosCall
        pop     ds
        endm

@RestoreIntVector macro nr,altvektor
        mov     ax,word ptr altvektor+2
        mov     dx,word ptr altvektor+0
        push    ds
        mov     ds,ax
        mov     ah,25h
        mov     al,nr
        @DosCall
        pop     ds
        endm

        popcontext listing
