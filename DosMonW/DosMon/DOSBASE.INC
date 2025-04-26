        pushcontext listing
        .xlist
        .xcref

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

        popcontext listing
