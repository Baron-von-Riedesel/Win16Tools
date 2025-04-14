
#include <xlistbox.h>
                                                    /* tabellengroesse */
#define TABDIM 2000

extern HINSTANCE hInst;                              /* handle instance */
extern HMENU     hMenu;                              /* handle menu (menu wird modifiziert) */
extern HWND      hWndMain;                           /* handle main window */

typedef struct tagNOTSTR {
    WORD  wID;
    DWORD dwData;
    HTASK hTask;
    char  szTask[9];
    char  szModule[9];
    union {
     WORD  w1;
     HMODULE hModule;
     };
   } NOTSTR;

typedef NOTSTR FAR *LPNOTSTR;

typedef struct tagNOTSTRUCT
  {
   int      intnr;
   int      TimerInterval;       // refresh intervall
   BOOL     fSaveOptions;        // sollen optionen gesichert werden?
   BOOL     fMonActive;          // monitor aktiv?
   BOOL     fDbgOutput;          // ausgabe auf debugterminal?
   BOOL     fIntReg;             // interruptregister installieren?
   BOOL     fIntRegistered;      // interruptregister installiert?

   HGLOBAL  hGMem;               // Handle des Speicherblocks
   HGLOBAL  hStrings;
   LPSTR    lpStrings;
   int      anzstr;
   LPNOTSTR lpns;                // zeigt konstant auf start der tabelle
   WORD     wNFYIndex;           // NotifyProc: aktueller index der tabelle
   WORD     wNFYItems;           // NotifyProc: neue Eintraege
   WORD     wActItem;            // aktueller Eintrag in Tabelle
   WORD     wNumItems;           // Anzahl Eintraege in Tabelle
   BOOL     fAktTab[14];
  } NOTSTRUCT;

