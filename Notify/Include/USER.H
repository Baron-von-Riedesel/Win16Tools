
#ifndef _INC_USER
#define _INC_USER

#ifdef _INC_WINDOWS
#define __WINDOWS_H
#endif

#define IsModule(x) (*(LPINT)MAKELONG(0,x) == 0x454E)

#ifndef MK_FP
#define MK_FP(sel, off)     ((void FAR*)MAKELONG((off), (sel)))
#endif

typedef void                *PVOID;

/* allgemeine funktionen */

#define MulDivWords(x,y,z) (DWORD)x*(DWORD)y/z

#ifndef NO_EXT_KEYS
    #define _CDECL  cdecl
#else
    #define _CDECL
#endif

/* typdefinitionen */

typedef char __huge         *HPSTR;
typedef const char __huge   *HPCSTR;
typedef BYTE __huge         *HPBYTE;
typedef const BYTE __huge   *HPCBYTE;
typedef WORD __huge         *HPWORD;
typedef void __huge         *HPVOID;

#define CODESEG _based(_segname("_CODE"))

typedef const char _based(_segname("_CODE")) CSTRC,*PCSTRC;

typedef struct tagFWORD {
        DWORD offset;
        WORD selector;
       } FWORD;

typedef FWORD FAR * LPFWORD;

typedef unsigned short SELECTOR;

typedef struct tagDESCRIPTOR
   {
     WORD limit1;
     WORD base1;
     BYTE base2;
     BYTE attr;
     unsigned char limit:4,attrh:4;
     BYTE base3;
   } DESCRIPTOR;

typedef DESCRIPTOR FAR * LPDESCRIPTOR;

typedef struct tagIDTENTRY
   {
     WORD id;
     DWORD offset;
     WORD selector;
     WORD attr;
     WORD wLimit;
     DWORD dwAddr;
   } IDTENTRY;

typedef IDTENTRY FAR * LPIDTENTRY;

typedef struct tagGDTENTRY
   {
     WORD selector;
     DWORD base;
     DWORD limit;
     WORD attr;
   } GDTENTRY;

typedef GDTENTRY FAR * LPGDTENTRY;

typedef struct tagIOBMENTRY
   {
     WORD selector;
     WORD limit;
     WORD base;
     WORD offset;
     WORD bit;
     WORD flag;
     WORD port;
   } IOBMENTRY;

typedef IOBMENTRY FAR * LPIOBMENTRY;

typedef struct tagLDTENTRY
   {
     WORD ldtselector;
     WORD ldtlimit;
     WORD selector;
     DWORD base;
     DWORD limit;
     WORD attr;
   } LDTENTRY;

typedef LDTENTRY FAR * LPLDTENTRY;

typedef struct tagPAGEENTRY
  {
   WORD  wEntry;          /* Entry */
   DWORD dwPage;          /* phys Page */
   WORD  wFlags;          /* Flags */
  } PAGEENTRY;

#ifdef _FIND_T_DEFINED
typedef struct _find_t _far * LPFINDT;
#else
typedef void _far * LPFINDT;
#endif

typedef struct tagENUMFILESTRUCT {
    LPSTR   lpszPath;
#ifdef __cplusplus
    BOOL (FAR PASCAL *fpCallbackFiles)(tagENUMFILESTRUCT FAR *,LPSTR,LPFINDT);
    BOOL (FAR PASCAL *fpCallbackDirs)(tagENUMFILESTRUCT FAR *,LPSTR,LPFINDT);
#else
    BOOL (FAR PASCAL *fpCallbackFiles)(LPENUMFILESTRUCT,LPSTR,LPFINDT);
    BOOL (FAR PASCAL *fpCallbackDirs)(LPENUMFILESTRUCT,LPSTR,LPFINDT);
#endif
    WORD    wTypes;      // welche Eintraege sollen aufgelistet werden?
    WORD    wNumFiles;
    WORD    wDirectories;
    DWORD   dwSizeOfDirs;
    WORD    wClSize;
    WORD    wDirEntries;
    WORD    wDirLevel;
   } ENUMFILESTRUCT,FAR * LPENUMFILESTRUCT;

typedef struct tagEXEC {    // Parameter EXEC (ax=4B00)
   unsigned segEnv;
   LPSTR    lpParm;
   LPVOID   lpFCB1;
   LPVOID   lpFCB2;
           } EXECPARM;

/* funktions prototypen */

typedef struct tagPUSHAREGS   /* registerbelegung bei pusha */
  {
    WORD       rdi;
    WORD       rsi;
    WORD       rbp;
    WORD       rsp;
    WORD       rbx;
    WORD       rdx;
    WORD       rcx;
    WORD       rax;
  } PUSHAREGS;

typedef struct tagPUSHADREGS   /* registerbelegung bei pushad */
  {
    DWORD       redi;
    DWORD       resi;
    DWORD       rebp;
    DWORD       resp;
    DWORD       rebx;
    DWORD       redx;
    DWORD       recx;
    DWORD       reax;
  } PUSHADREGS;

#ifdef __cplusplus
extern "C" {
#endif

int          _CDECL _setalarm(long int,char *);
long int     _CDECL _resetalarm(void);
void     FAR pascal DosBeep(int,int);                 /* tonhohe,dauer   */

unsigned     _CDECL _GetPMode(void);                   /* processor mode (0,1,2) */

unsigned  pascal _GetSegWord(unsigned,unsigned);       /* wort aus unsigned:unsigned holen */

char *       _CDECL  strcatc(char *,int);  /* P2 an P1 anfgen           */

int          _CDECL  lstrncpy(char far *,char far *,int); /* strncpy mit far pointern   */
int      FAR PASCAL  _lmemcpy (LPBYTE,LPBYTE,WORD);
int          _CDECL  lstrncat(char far *,char far *,int); /* P2 an P1 anfgen           */
int          _CDECL  _lstrcmp(LPCSTR,LPCSTR);             /* modellspezifisches strcmp */
int          _CDECL  _lstrncmp(LPCSTR,LPCSTR,WORD);       /* modellspezifisches strncmp */

DWORD    FAR PASCAL  _hstrlen (HPCSTR);                 /* gibt eigentlich DWORD zurueck */
int      FAR PASCAL  _hstrcmp (HPCSTR,HPCSTR);          /* */

int          _CDECL  _setport(int,int,int);            /* ausgabe byte auf "indexport"  */
void         _CDECL  _ExecInt28(void);                 /* nichts zu tun, INT 28 aufrufen */
char *       _CDECL  xtoa(unsigned,char *);            /* P1 in string (hex) umwandeln   */
char *       _CDECL  utoa(unsigned,char *);            /* P1 in string (dez) umwandeln   */
char *       _CDECL  _axtou(char *,unsigned *);        /* P1 in unsigned umwandeln   */
void far* FAR _CDECL _GetStartofMCBs(void);            /* anfang der MCB-Kette */
int          _CDECL  _GetOwner(char far *,char *);     /* owner aus environment holen */
LPSTR        _CDECL  _ConvByteHex(LPSTR,char);         /* byte -> string */
char *       _CDECL  _ConvDWordDez(char *,unsigned long,int);  /* DWORD -> string */
unsigned     _CDECL  _loadprg(char *,char *,int);      /* programm laden, nicht ausfhren */
WORD         _CDECL  GetMyDS();

WORD         pascal  xsetjmp(void *);                  /* 9 register */
WORD         _CDECL  xlongjmp(void *,WORD);

HANDLE   FAR PASCAL RegisterNotPresentHandler(WORD, FARPROC);
BOOL     FAR PASCAL UnregisterNotPresentHandler(WORD);
HANDLE   FAR PASCAL RegisterStackErrorHandler(WORD, FARPROC);
BOOL     FAR PASCAL UnregisterStackErrorHandler(WORD);
HANDLE   FAR PASCAL RegisterProtectionHandler(WORD, FARPROC);
BOOL     FAR PASCAL UnregisterProtectionHandler(WORD);
HANDLE   FAR PASCAL RegisterPageErrorHandler(WORD, FARPROC);
BOOL     FAR PASCAL UnregisterPageErrorHandler(WORD);
BOOL     FAR PASCAL RegisterExceptionHandler(WORD,int (FAR *)(), LPVOID);
BOOL     FAR PASCAL UnregisterExceptionHandler(WORD, LPVOID);

WORD     FAR PASCAL GetLastException(void);
BOOL     FAR PASCAL RegisterKeyboardDispatcher(FARPROC);
BOOL     FAR PASCAL UnregisterKeyboardDispatcher(void);

WORD     FAR PASCAL IsValidSelector(WORD);             // LAR ausfuehren
WORD     FAR PASCAL IsPresent(WORD);                   // LAR + test auf NP
WORD     FAR PASCAL IsValidAddress(LPVOID);            // LAR + SEL -> ES
WORD     FAR PASCAL IsReadableSelector(WORD);          // VERR ausfuehren
BOOL     FAR PASCAL IsMemoryDescriptor(WORD);
WORD     FAR PASCAL AllocDescriptor(DWORD,WORD);       /* basis, limit */
WORD     FAR PASCAL FreeDescriptor(WORD);
WORD     FAR PASCAL AllocRMDescriptor(WORD);           /* segment */
BOOL     FAR PASCAL GetDescriptor(WORD,LPDESCRIPTOR);
BOOL     FAR PASCAL SetDescriptor(WORD,DWORD,DWORD,WORD);  /* selector,basis,limit,attr */
BOOL     FAR PASCAL GetBaseofSelector(WORD,LPDWORD);
BOOL     FAR PASCAL GetLimitofSelector(WORD,LPDWORD);
BOOL     FAR PASCAL GetAttrofSelector(WORD,UINT FAR *);
WORD     FAR PASCAL AdjustDescriptorAttr(WORD);

DWORD    FAR PASCAL Unassemble(FWORD,LPSTR,WORD);
void     FAR        lqsort(void FAR *,WORD,WORD,FARPROC);
void                lqsorts(void FAR *,WORD,WORD,NEARPROC);
int      FAR PASCAL _hqsortlp(void FAR *,DWORD);
void     FAR PASCAL GetDescriptorType(WORD,LPSTR);
void     FAR PASCAL WaitTime(WORD);
WORD     FAR PASCAL GetCPL(void);
VOID     FAR PASCAL _strout(LPSTR);
DWORD    FAR PASCAL _GetFileSize(LPSTR);
VOID     FAR PASCAL DebugBreak(void);
VOID     FAR PASCAL _lwriteline(int,LPSTR);
BOOL     FAR PASCAL CreateRegister(LPSTR,DWORD,LPSTR);
BOOL     FAR PASCAL EnumFiles(LPENUMFILESTRUCT);
LPVOID   FAR PASCAL GetSDAAddr();
WORD     FAR PASCAL GetLDTSelector();
WORD     FAR PASCAL GetTrueVersion();
int  __cdecl _fcputs(const char _far *);
#ifndef _lopen
int      FAR PASCAL _lopen(const char _far *, int );
#endif

#ifdef __cplusplus
}
#endif

#endif

