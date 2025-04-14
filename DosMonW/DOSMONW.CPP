
#include "string.h"
#include "stdlib.h"
#include "windows.h"
//#include "toolhelp.h"
#include "user.h"
#include "userw.h"
#include "dosmonw.h"
#include "dosmonx.h"

#define ANZTAB 14
#define MAXDOSFN 0x72
#define TABDIM 1000
#define SELMODE 1

extern "C" {
// extern int qsum;
// extern int rqsum;
extern char _DOSTEXTE[MAXDOSFN][12];
};

BOOL CALLBACK AboutDlg    (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK RegisterDlg (HWND, UINT, WPARAM, LPARAM);
// BOOL CALLBACK FileDlg     (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK OptionDlg   (HWND, UINT, WPARAM, LPARAM);

typedef BOOL (CALLBACK *SPIFNPTR)(UINT,UINT,VOID FAR *,UINT);

LRESULT CALLBACK WndProc  (HWND, UINT, WPARAM, LPARAM);
WORD    CALLBACK TimerProc(HWND ,WORD, int,  DWORD);
LRESULT DoCommand   (HWND, WPARAM, LPARAM);
BOOL    InittheApp  (HINSTANCE);

HACCEL    hAccelTable;       // handle der accelerators
HWND      hWndMain;          // handle main window
HWND      hWndChild;         // handle child window
HINSTANCE hInst;             // handle instance
HFONT     tFont;             // handle des kreierten fonts
HMENU     hMenu;             // handle menu (menu wird modifiziert)
HHOOK     lpFilterFunc;
PBYTE     pStack;

int      regtype = IDC_TYPEP;                 /* registrierter typ (protected oder real) */
DWORD    selecs;                             /* segment/selektor des DOS-Memorys */
int      fSaveOptions = 0;                   /* flag: werden optionen gesichert? */
int      TimerInterval = 2000;               /* refresh intervall */
int      fMonaktiv = FALSE;                  /* flag: monitor aktiv? */
int      xpos = 100,ypos = 100;              /* window position */
int      xsize = 600,ysize = 200;            /* window extents */
int      fCarry = FALSE;
int      anzstr;

                                             /* pointer auf resource strings */
PSTR pszAppName,pszTitle,pszDatstr;
PSTR pszStart,pszStop,pszRMSwitch,pszIniName;
PSTR pszPMSwitch,pszNHSwitch,pszProtMode,pszRealMode,pszNoHookMode,pszInactiv;
PSTR pszWinCap;

int      charh;                              /* zeichenhoehe */
WORD     dosfn[16];
WORD     bitno[16] = {0x0001,0x0002,0x0004,0x0008,
                      0x0010,0x0020,0x0040,0x0080,
                      0x0100,0x0200,0x0400,0x0800,
                      0x1000,0x2000,0x4000,0x8000};


LPREGSTRUCT lpregstruct;            // zeigt konstant auf start der reg-tabelle
LPINT    lpi1;                      // zeigt auf aktuellen index fuer reg-tabelle
LPWORD   lpzaehler;                 // zeigt auf zaehler fuer reg-entries
WORD     lzaehler = 0;              // zaehler fuer "bekannte" reg-entries
                                    // copyright
int tabpos[ANZTAB];
int tabdiffs[ANZTAB+1] = {44,
                          108,               /* funktion */
                          44,                /* ax - dx */
                          44,
                          44,
                          44,
                          20,                /* carry */
                          44,                /* ax - dx */
                          44,
                          44,
                          44,
                          80,
                          80,
                          32000,
                          0
                         };
                                             /* tabelle fuer string load */
static STRLOADENTRY strloadtab[] = {
                       &pszAppName, IDS_APPNAME,
                       &pszTitle,   IDS_TITLE,
                       &pszDatstr,  IDS_DATE,
                       &pszStart,   IDS_START,
                       &pszStop,    IDS_STOP,
                       &pszRMSwitch,IDS_RMSWITCH,
                       &pszPMSwitch,IDS_PMSWITCH,
                       &pszNHSwitch,IDS_NHSWITCH,
                       &pszRealMode,IDS_RMSTR,
                       &pszProtMode,IDS_PMSTR,
                       &pszNoHookMode,IDS_NHSTR,
                       &pszIniName, IDS_ININAME,
                       &pszInactiv, IDS_INACTIV,
                       &pszWinCap,  IDS_WINCAP,
                       (PSTR *)(0)};
                                             // profile load tabelle
                                             // 0=nur lesen/1=auch schreiben
                                             // 2=bedingt schreiben
static PROFSTRUC proftab[] = {
                       1,0,"xPos",&xpos,
                       1,0,"xSize",&xsize,
                       1,0,"yPos",&ypos,
                       1,0,"ySize",&ysize,
                       0,0,"fn000F",(PINT)&dosfn[0],
                       0,0,"fn101F",(PINT)&dosfn[1],
                       0,0,"fn202F",(PINT)&dosfn[2],
                       0,0,"fn303F",(PINT)&dosfn[3],
                       0,0,"fn404F",(PINT)&dosfn[4],
                       0,0,"fn505F",(PINT)&dosfn[5],
                       0,0,"fn606F",(PINT)&dosfn[6],
                       0,0,"fn707F",(PINT)&dosfn[7],
                       0,0,"fn808F",(PINT)&dosfn[8],
                       0,0,"fn909F",(PINT)&dosfn[9],
                       0,0,"fnA0AF",(PINT)&dosfn[10],
                       0,0,"fnB0BF",(PINT)&dosfn[11],
                       0,0,"fnC0CF",(PINT)&dosfn[12],
                       0,0,"fnD0DF",(PINT)&dosfn[13],
                       0,0,"fnE0EF",(PINT)&dosfn[14],
                       0,0,"fnF0FF",(PINT)&dosfn[15],
                       2,0,"MonType",&regtype,
                       2,0,"Carry",&fCarry,
                       2,0,"TimerInterval",&TimerInterval,
                       1,0,"SaveOptions",&fSaveOptions,
                       0,0,(PSTR)0,NULL
                      };
static char * kaptab[] = {"parms"};

static LOGFONT mylogfont = {
                       8,                           // height
                       0,                           // width
                       0,                           // escapement
                       0,                           // orientation
                       700,                         // weight
                       FALSE,                       // italic
                       FALSE,                       // underline
                       FALSE,                       // strikeout
                       ANSI_CHARSET,                // charset
                       OUT_DEFAULT_PRECIS,          // out precision
                       CLIP_DEFAULT_PRECIS,         // clip precision
                       DEFAULT_QUALITY,             // quality
                       0,                           // pitch & family
                       "Helv"                       // Fontname
                      };
/////////////////////////////////////////////////////////////////////
int MyCreateMessage(HWND hWnd,UINT uiStringID)
/////////////////////////////////////////////////////////////////////
{
 char str[128];

 LoadString(hInst,uiStringID,str,sizeof(str));
 MessageBox(hWnd,str,0,MB_OK);

 return 1;
}
///////////////////////////////////////////////////////////////////
// WriteLine schreibt eine Zeile in Snapshot Datei
///////////////////////////////////////////////////////////////////
void WriteLine(HWND hWnd, int hFile, int itemID)
{
  char str[128];
  char str1[40];
  char str2[20];
  char str3[10];
  int i;
  HTASK hTask;
  char fchar;
  WORD zax;
  WORD aktzeile;
  LPREGSTRUCT lpr;
  PINT pint;
  int  inttab[20];

                                           /* i -> logischer index 0 in tabelle */
  i  = *lpi1 - (*lpzaehler - lzaehler) - 1;
  i = i - itemID;
  if (i < 0)
      i = i+TABDIM;

  lpr = lpregstruct + i;
  zax = lpr->pax;
                                           /* schluesseluebersetzung holen */
  if  (HIBYTE(lpr->pax) < MAXDOSFN) {
      strncpy(str1,_DOSTEXTE[HIBYTE(lpr->pax)],12);
      str1[12] = 0;
  } else
      if  (HIBYTE(lpr->pax) == 0xff) {
          strcpy(str1,"ReturnPrg");
          zax = 0x4b00 | LOBYTE(zax);
      } else
          strcpy(str1,"???");
                                       /* carry flag aufbereiten */
  if (lpr->flags & 1)
      fchar = 'C';
  else
      fchar = ' ';

  aktzeile = (WORD)(lzaehler - itemID);
                                       /* im real mode die register anders aufbereiten */
	if (lpr->user1 == 0xFFFF) {
		switch (lpr->user2) {
		case IDC_TYPER:
			strcpy(str1,pszRMSwitch);
			break;          
		case IDC_TYPEP:
			strcpy(str1,pszPMSwitch);
			break;
		case IDC_TYPENH:
			strcpy(str1,pszNHSwitch);
			break;
		default:         
			wsprintf(str1,"switch to %X",lpr->user2);	//ist eigentlich Fehler
			break;
	}   

      wsprintf(str,"%u\t\t\t\t----------\t%s\t----------\t\t\t\t\t",
               aktzeile,
               (LPSTR)str1
              );
  } else {
      if (((HIBYTE(lpr->pax) >= 0x39) && (HIBYTE(lpr->pax) <= 0x3D))
       || (HIBYTE(lpr->pax) == 0x41)
       || (HIBYTE(lpr->pax) == 0x43)
       || (HIBYTE(lpr->pax) == 0x4B)
       || (HIBYTE(lpr->pax) == 0x4E)
       || (HIBYTE(lpr->pax) == 0x5B) ) {
          _fstrncpy(str2,(LPSTR)&(lpr->res1),12);
          _memxchg(str2,0,' ',12);
          str2[12] = 9;
          str2[13] = 0;
//          wsprintf(str3,"%X",lpr->ss);
      } else {
          wsprintf(str2,"%04X\t%04X", lpr->dx,lpr->ds);
//          if (lpr->user1 != 0)
//              wsprintf(str3,"%04X",lpr->user1);
      }
      if (hTask = GetTaskFromInstance((HINSTANCE)lpr->ss))
          GetTaskName(hTask,str3,sizeof(str3));
      else
          wsprintf(str3,"%X",lpr->ss);


      pint = inttab;
      *pint++ = aktzeile;                  /* zeilennummer */
      *pint++ = LOWORD((LPSTR)&str1);             /* funktion */
      *pint++ = HIWORD((LPSTR)&str1);
      *pint++ = zax;
      *pint++ = lpr->pbx;
      *pint++ = lpr->pcx;
      *pint++ = lpr->pdx;
      *pint++ = fchar;
      *pint++ = lpr->ax;
      *pint++ = lpr->bx;
      *pint++ = lpr->cx;
      *pint++ = LOWORD((LPSTR)&str2);
      *pint++ = HIWORD((LPSTR)&str2);
      *pint++ = lpr->cs;
      *pint++ = lpr->ip;
      *pint++ = LOWORD((LPSTR)&str3);             /* task */
      *pint++ = HIWORD((LPSTR)&str3);

      wvsprintf(str,
                "%u\t%s\t%04X\t%04X\t%04X\t%04X\t%c\t%04X\t%04X\t%04X\t%s\t%04X:%04X\t%s\t",
                (LPSTR)&inttab
               );
     }

  strcat(str,"\r\n");
  _lwrite(hFile,str,strlen(str));

}
/////////////////////////////////////////////////////////////////////
int WriteSnapShot(int hFile, HWND hWnd, LPCSTR cstrg, WORD mode)
/////////////////////////////////////////////////////////////////////
{
 char  szStr[65];
 char  str[100];
 UINT   i;

   szStr[0] = 0;
   if (!hFile) {
       GetTempFileName(0,cstrg,0,szStr);
       hFile = _lcreat(szStr,0);
   }
   if (hFile == 0xFFFF)
       return 0;

   SetCursor(LoadCursor(0,IDC_WAIT));

   if (*pszWinCap) {
       _lwrite(hFile,pszWinCap,lstrlen(pszWinCap));
       str[0] = 0x0D;
       str[1] = 0x0A;
       _lwrite(hFile,str,2);
   }
   for (i = 0 ; i < lzaehler ; i++)
       WriteLine(hWnd,hFile,i);

   _lclose(hFile);

   SetCursor(LoadCursor(0,IDC_ARROW));

   wsprintf(str,"Gespeichert in\n%s",(LPSTR)szStr);
   MessageBox(hWndMain,str,"Hinweis",MB_OK);

   return hFile;
}
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ aus Tabellenzeile index ermitteln                                  บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
int searchline(WORD aktzeile)
{
  int i;
  i  = *lpi1 - (*lpzaehler - lzaehler) - 1;
  i = i - aktzeile;
  if (i < 0)
      i = i+TABDIM;

  return i;
}
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ IntProc - Monitor (nur protected mode)                             บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
void FAR PASCAL IntProc(REGSTRUCT FAR * lpregs)
{
 LPREGSTRUCT lpr;

 if (fCarry && ((lpregs->flags & 1) == 0))
     return;
#if SELMODE
 if (dosfn[lpregs->pax>>12] & bitno[(lpregs->pax>>8) & 0x0F])
#else
 if ((HIBYTE(lpregs->pax) == 0x3D) ||
     (HIBYTE(lpregs->pax) == 0x6C) ||
     (HIBYTE(lpregs->pax) == 0x3E) ||
     (HIBYTE(lpregs->pax) == 0x3F) ||
     (HIBYTE(lpregs->pax) == 0x40) ||
     (HIBYTE(lpregs->pax) == 0x3C) )
#endif
    {
     lpr = lpregstruct + *lpi1;
     *lpr = *lpregs;
     if (!(lpr->user1))
         lpr->user1 = (WORD)GetCurrentTask();
     (*lpi1)++;
     if (*lpi1 > TABDIM-1)
        *lpi1 = 0;
     (*lpzaehler)++;
    }
}
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ Dummy Eintrag in Tabelle einfuegen                                 บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
void AddEntry(int x)
{
 LPREGSTRUCT lpr;

 lpr = lpregstruct + *lpi1;
 lpr->user1 = 0xFFFF;
 lpr->user2 = x;
 (*lpi1)++;
 if (*lpi1 > TABDIM-1)
    *lpi1 = 0;
 (*lpzaehler)++;
// TimerProc(hWndMain,0,0,0);
}
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ TimerProc                                                          บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
WORD EXPORTED CALLBACK TimerProc(HWND hWnd,WORD wMsg,int idEvent,DWORD dwtime)
{
 int diff;
 int diffstr;
 WORD index;
 WORD selec;
 WORD anzzeilen;

                                   /* anzahl der inzwischen neu hinzugekommenen eintraege */
 diff = *lpzaehler - lzaehler;

 if (diff) {
    if (anzstr < TABDIM) {
        if ((anzstr + diff) > TABDIM) {
            diffstr = TABDIM - anzstr;
            anzstr = TABDIM;
        } else {
            anzstr = anzstr + diff;
            diffstr = diff;
        }
        SendMessage(hWndChild,XLB_ADDMULTSTRINGS,diffstr,0);
    }

    index = (WORD)SendMessage(hWndChild,LB_GETTOPINDEX,0,0);
    selec = (WORD)SendMessage(hWndChild,LB_GETCURSEL,0,0);
    anzzeilen = (WORD)SendMessage(hWndChild,XLB_GETLINES,0,0);
                                                         /* nonstandard lParam */
    if ((index + diff) >= (TABDIM - anzzeilen)) {
        SendMessage(hWndChild,XLB_CHANGECURPOS,TABDIM - anzzeilen,0);
        InvalidateRect(hWndChild,0,0);
    } else
        SendMessage(hWndChild,XLB_CHANGECURPOS,index + diff,0);

    if (!index)
        SendMessage(hWndChild,WM_VSCROLL,SB_TOP,0);

    lzaehler = *lpzaehler;
 }

 return 1;
}
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ char height eines fonts holen                                      บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
int GetCharHeight(HWND hWnd,HFONT hfont)
{
  TEXTMETRIC metrics;
  HDC hdc;

  hdc = GetDC(hWnd);
  SelectObject(hdc,hfont);
  GetTextMetrics(hdc,&metrics);
  ReleaseDC(hWnd,hdc);
  return metrics.tmHeight;
}
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ Registerit - Monitor registrieren                                  บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
int Registerit(HWND hWnd, int wReqType)
{
	int rc;
	char str[80];

	if (!pStack)
		pStack = (PBYTE)LocalAlloc(LMEM_FIXED,0x400) + 0x400;

	if (!pStack)
		return 0;

	if ((wReqType == IDC_TYPEP) || (wReqType == IDC_TYPENH))
		rc = RegisterDosMon(0x21,(FARPROC)IntProc,wReqType - IDC_TYPEP,(WORD)pStack);
	else
		rc = RegisterDosMon(0x21,(FARPROC)MAKELP(HIWORD(selecs),0),wReqType - IDC_TYPEP,(WORD)pStack);

	if (rc) {
		switch (wReqType) {
		case IDC_TYPEP:
			wsprintf(str,"%s (%s)",(LPSTR)pszTitle,(LPSTR)pszProtMode);
			break;
		case IDC_TYPENH:
			wsprintf(str,"%s (%s)",(LPSTR)pszTitle,(LPSTR)pszNoHookMode);
			break;
		case IDC_TYPER:
			wsprintf(str,"%s (%s)",(LPSTR)pszTitle,(LPSTR)pszRealMode);
			break;
		default:         
			wsprintf(str,"%s (%s)",(LPSTR)pszTitle,(LPSTR)"???");
			break;
		}    
//		OutputDebugString("RegisterIt ");
//		OutputDebugString(str);
//		OutputDebugString("\r\n");
		regtype = wReqType;
		SetWindowText(hWndMain,str);
	}
	return rc;
}
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ Paintroutine                                                       บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
void DoPaint(HWND hWnd,DRAWITEMSTRUCT FAR * lpdistruct)
{
  char str[128];
  char str1[40];
  char str2[20];
  char str3[10];
  int i;
  HTASK hTask;
  char fchar;
  WORD zax;
  WORD aktzeile;
  LPREGSTRUCT lpr;
  PINT pint;
  int  inttab[20];

                                           /* i -> logischer index 0 in tabelle */
  i  = *lpi1 - (*lpzaehler - lzaehler) - 1;
  i = i - lpdistruct->itemID;
  if (i < 0)
      i = i+TABDIM;

  lpr = lpregstruct + i;
  zax = lpr->pax;
                                           /* schluesseluebersetzung holen */
  if  (HIBYTE(lpr->pax) < MAXDOSFN)
     {
      strncpy(str1,_DOSTEXTE[HIBYTE(lpr->pax)],12);
      str1[12] = 0;
     }
  else
  if  (HIBYTE(lpr->pax) == 0xff)
     {
      strcpy(str1,"ReturnPrg");
      zax = 0x4b00 | LOBYTE(zax);
     }
  else
      strcpy(str1,"???");
                                       /* carry flag aufbereiten */
  if (lpr->flags & 1)
      fchar = 'C';
  else
      fchar = ' ';

  aktzeile = (WORD)(lzaehler - lpdistruct->itemID);
                                       /* im real mode die register anders aufbereiten */
	if (lpr->user1 == 0xFFFF) {
		switch (lpr->user2) {
		case IDC_TYPER:
			strcpy(str1,pszRMSwitch);
			break;
		case IDC_TYPEP:
			strcpy(str1,pszPMSwitch);
			break;
		case IDC_TYPENH:
			strcpy(str1,pszNHSwitch);
			break;
		default:
			wsprintf(str1,"switch to %X",lpr->user2);
			break;
		}
//		OutputDebugString("DoPaint >");
//		OutputDebugString(str1);
//		OutputDebugString("<\r\n");
		wsprintf(str,"%u\t\t\t\t----------\t%s\t----------\t\t\t\t",
               aktzeile,
               (LPSTR)str1
              );
	} else {
		if (((HIBYTE(lpr->pax) >= 0x39) && (HIBYTE(lpr->pax) <= 0x3D))
		|| (HIBYTE(lpr->pax) == 0x41)
		|| (HIBYTE(lpr->pax) == 0x43)
		|| (HIBYTE(lpr->pax) == 0x4B)
		|| (HIBYTE(lpr->pax) == 0x4E)
		|| (HIBYTE(lpr->pax) == 0x5B) ) {
			_fstrncpy(str2,(LPSTR)&(lpr->res1),12);
			_memxchg(str2,0,' ',12);
			str2[12] = 0;
//          wsprintf(str3,"%X",lpr->ss);
		} else {
			wsprintf(str2,"%04X\t%04X", lpr->dx,lpr->ds);
//          if (lpr->user1 != 0)
//              wsprintf(str3,"%04X",lpr->user1);
		}
		if (hTask = GetTaskFromInstance((HINSTANCE)lpr->ss))
			GetTaskName(hTask,str3,sizeof(str3));
		else
			wsprintf(str3,"%X",lpr->ss);


		pint = inttab;
		*pint++ = aktzeile;                  /* zeilennummer */
		*pint++ = LOWORD((LPSTR)&str1);             /* funktion */
		*pint++ = HIWORD((LPSTR)&str1);
		*pint++ = zax;
		*pint++ = lpr->pbx;
		*pint++ = lpr->pcx;
		*pint++ = lpr->pdx;
		*pint++ = fchar;
		*pint++ = lpr->ax;
		*pint++ = lpr->bx;
		*pint++ = lpr->cx;
		*pint++ = LOWORD((LPSTR)&str2);
		*pint++ = HIWORD((LPSTR)&str2);
		*pint++ = lpr->cs;
		*pint++ = lpr->ip;
		*pint++ = LOWORD((LPSTR)&str3);             /* task */
		*pint++ = HIWORD((LPSTR)&str3);

		wvsprintf(str,
                "%u\t%s\t%04X\t%04X\t%04X\t%04X\t%c\t%04X\t%04X\t%04X\t%s\t%04X:%04X\t%s\t",
                (LPSTR)&inttab);
	}

	if (lpdistruct->itemState & ODS_SELECTED) {
		SetBkColor(lpdistruct->hDC,GetSysColor(COLOR_HIGHLIGHT));
		SetTextColor(lpdistruct->hDC,GetSysColor(COLOR_HIGHLIGHTTEXT));
	}

	TabbedTextOut(lpdistruct->hDC,lpdistruct->rcItem.left,lpdistruct->rcItem.top,str,strlen(str),ANZTAB,tabpos,0);

	if (lpdistruct->itemState & ODS_FOCUS)
		DrawFocusRect(lpdistruct->hDC,&lpdistruct->rcItem);

	if (lpdistruct->itemState & ODS_SELECTED) {
		SetBkColor(lpdistruct->hDC,GetSysColor(COLOR_WINDOW));
		SetTextColor(lpdistruct->hDC,GetSysColor(COLOR_WINDOWTEXT));
	}
}
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ Paint Caption                                                      บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
void PaintCaption(HWND hWnd,HDC hdc,PRECT prect)
{

  if (prect->top < charh)
     {
      SelectObject(hdc,tFont);
      SetBkColor(hdc,GetSysColor(COLOR_WINDOW));
      SetTextColor(hdc,GetSysColor(COLOR_MENUTEXT));
      TabbedTextOut(hdc,0,0,pszWinCap,strlen(pszWinCap),ANZTAB,tabpos,0);
      MoveTo(hdc,0,charh-1);
      LineTo(hdc,prect->right,charh-1);
     }
}
///////////////////////////////////////////////////////////////////
int SetActivMode(HWND hWnd,int wMode)
{
  char str[80];

  if (wMode) {
      ModifyMenu(hMenu,IDM_STARTSTOP,MF_BYCOMMAND,IDM_STARTSTOP,pszStop);
      fMonaktiv = TRUE;
  } else {
      ModifyMenu(hMenu,IDM_STARTSTOP,MF_BYCOMMAND,IDM_STARTSTOP,pszStart);
      wsprintf(str,"%s (%s)",(LPSTR)pszTitle,(LPSTR)pszInactiv);
      SetWindowText(hWndMain,str);
      fMonaktiv = FALSE;
  }
  return 0;
}
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ DoCommand - WM_COMMAND verarbeiten                                 บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
LRESULT DoCommand(HWND hWnd,WPARAM wParam,LPARAM lParam)
{
  int len;
  PSTR pstr;
  int newtype;
  WORD index;
  char str[80];
  LRESULT rc = 0;

  switch (wParam) {
    case IDM_MAINLB:
      switch (HIWORD(lParam)) {
         case LBN_DBLCLK:
            PostMessage(hWnd,WM_COMMAND,IDM_REGSOUT,0);
            break;
      }
      break;
    case IDM_EXIT:
      PostMessage(hWnd,WM_CLOSE,0,0);
      break;
    case IDM_HELP:
      len = GetModuleFileName(hInst,str,sizeof(str));
      pstr = str + len - 3;
      strcpy(pstr,"hlp");
      WinHelp(hWnd,str,HELP_INDEX,0);
      break;
    case IDM_CLEAR:
      *lpi1      = 0;
      *lpzaehler = 0;
      lzaehler   = 0;
      anzstr     = 0;
      SendMessage(hWndChild,LB_RESETCONTENT,0,0);
      break;
    case IDM_SAVE:
      newtype = fMonaktiv;
      if (newtype)
         SendMessage(hWnd,WM_COMMAND,IDM_STARTSTOP,0);
      WriteSnapShot(0,hWndChild,"DM",5);
      if (newtype)
         SendMessage(hWnd,WM_COMMAND,IDM_STARTSTOP,0);
      break;
    case IDM_INTVCHANGE:
      KillTimer(hWnd,1);
      SetTimer(hWnd,1,TimerInterval,(TIMERPROC)TimerProc);
      break;
    case IDM_TYPECHANGE: 
      if (LOWORD(lParam) != 0) {		// wenn von Dialogbox gesendet
       	  newtype = LOWORD(lParam);
		wsprintf(str,"new type is %X\r\n",LOWORD(lParam));
//		OutputDebugString(str);
      } else
	      switch (regtype) {
    	  case IDC_TYPEP:
			newtype = IDC_TYPENH;
			break;
    	  case IDC_TYPENH:
			newtype = IDC_TYPER;
			break;
    	  case IDC_TYPER:
			newtype = IDC_TYPEP;
			break;
    	  }    

      if (fMonaktiv)
          UnregisterDosMon(0x21,regtype - IDC_TYPEP);

      if (Registerit(hWnd,newtype)) {
          fMonaktiv = TRUE;
          AddEntry(newtype);
          rc = 1;
      } else {
          MyCreateMessage(hWnd,IDS_ERRSTART);
          if (fMonaktiv)
              if (!Registerit(hWnd,regtype))
                  SetActivMode(hWnd,0);
      }

      break;
    case IDM_ABOUT:
      CreateDialog(hInst,MAKEINTRESOURCE(IDD_ABOUTBOX),hWnd,AboutDlg);
      break;
    case IDM_OPTION:
      CreateDialog(hInst,MAKEINTRESOURCE(IDD_OPTIONBOX),hWnd,OptionDlg);
      break;
    case IDM_REGSOUT:
      if ((index = (WORD)SendMessage(hWndChild,LB_GETCURSEL,0,0)) != LB_ERR)
         CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_REGBOX),hWnd,RegisterDlg,(LONG)index);
      break;
//  case ID_FILES:
//    EnableMenuItem(hMenu,ID_FILES,MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
//    CreateDialog(hInst,MAKEINTRESOURCE(IDD_FILEBOX),0,FileDlg);
//    break;
    case IDM_STARTSTOP:
      if (fMonaktiv)
          if (UnregisterDosMon(0x21,regtype - IDC_TYPEP))
              SetActivMode(hWnd,0);
          else
              MyCreateMessage(hWnd,IDS_ERRSTOP);
      else
          if (Registerit(hWnd,regtype))
              SetActivMode(hWnd,1);
          else
              MyCreateMessage(hWnd,IDS_ERRSTART);
      break;
    default:
      break;
    }
 return rc;
}
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ WndProc - Windowroutine                                            บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
LRESULT CALLBACK WndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
  PAINTSTRUCT  ps;
  int rflag;
  PSTR pstr;
  PINT pint1,pint2;
  char str[80];
  RECT rect;
  int len;
  long rc;

    rc = FALSE;
    switch (message) {
    case WM_CREATE:

		hWndMain = hWnd;
        charh = GetCharHeight(hWnd,tFont);
        SetTimer(hWnd,1,TimerInterval,(TIMERPROC)TimerProc);
        rflag = Registerit(hWnd,regtype);
        pint1 = tabdiffs;
        pint2 = tabpos;
        *pint2++ = *pint1++;
        while (*pint1)
            *pint2++ = *(pint2-1) + *pint1++;

        if (rflag)
           {
            fMonaktiv = TRUE;
            AdjustClientSize(hWnd,0,10 * charh,0);
            hWndChild = CreateWindow("xlistbox",
                                     "",
                                     WS_CHILD | WS_VISIBLE | LBS_OWNERDRAWFIXED | LBS_WANTKEYBOARDINPUT,
                                     0,
                                     charh,
                                     100,
                                     100,
                                     hWnd,
                                     (HMENU)IDM_MAINLB,
                                     hInst,
                                     0
                                    );
            SendMessage(hWndChild,WM_SETFONT,(WPARAM)tFont,0);
           }
        else
           switch (regtype) {
           case IDC_TYPEP:
               MyCreateMessage(hWnd,IDS_ERRPMON);
               break;
           case IDC_TYPER:
               MyCreateMessage(hWnd,IDS_ERRRMON);
               break;
           case IDC_TYPENH:
               MyCreateMessage(hWnd,IDS_ERRNHMON);
               break;
           }    
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_CLOSE:
        if (fMonaktiv)
            if (!UnregisterDosMon(0x21,regtype - IDC_TYPEP))
               {
                MyCreateMessage(hWnd,IDS_ERRNOCLEX);
                break;
               }
        KillTimer(hWnd,1);
        len = GetModuleFileName(hInst,str,sizeof(str));
        pstr = str + len - 3;
        strcpy(pstr,"hlp");
        WinHelp(hWnd,str,HELP_QUIT,0);
        DestroyWindow(hWndMain);
        break;
    case WM_ACTIVATE:
        if (wParam)
            SetFocus(hWndChild);
        break;
    case WM_CHARTOITEM:
        if ((wParam == ' ') || (wParam == 13))
            PostMessage(hWnd,WM_COMMAND,IDM_REGSOUT,0);
        break;
#if 0 /* xlistbox class won't send WM_MEASUREITEM! */
    case WM_MEASUREITEM:
        OutputDebugString("WM_MEASUREITEM received\r\n");
        break;
#endif
    case WM_DRAWITEM:
        DoPaint(hWnd,(DRAWITEMSTRUCT FAR *)lParam);
        break;
    case WM_PAINT:
        BeginPaint(hWnd, (LPPAINTSTRUCT)&ps);
        PaintCaption(hWnd,ps.hdc,&ps.rcPaint);
        EndPaint(hWnd, (LPPAINTSTRUCT)&ps);
        break;
    case WM_SYSCOMMAND:
        rc = DefWindowProc(hWnd, message, wParam, lParam);
        break;
    case WM_COMMAND:
        rc = DoCommand(hWnd,wParam,lParam);
        break;
    case WM_SIZE:                                       /* fenstergroesse merken */
        if (wParam == SIZENORMAL)
          {
           GetWindowRect(hWnd,&rect);
           xsize = rect.right - rect.left;
           ysize = rect.bottom - rect.top;
          }
        if (HIWORD(lParam) % charh)
            AdjustClientSize(hWnd,0,(HIWORD(lParam) / charh) * charh,1);
        else
            MoveWindow(hWndChild,0,charh,LOWORD(lParam),HIWORD(lParam) - charh,TRUE);
        break;
    case WM_MOVE:                                       /* fensterposition merken */
        if (!IsIconic(hWnd))
          {
           GetWindowRect(hWnd,&rect);
           xpos = rect.left;
           ypos = rect.top;
          }
        break;
    default:
        rc = DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }
    return rc;
}
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ FilterFunc - Message Filter (fuer globales F1)       บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
BOOL CALLBACK FilterFunc(int nCode,WORD wParam,DWORD lParam )
{
 LPMSG lpmsg = (LPMSG)lParam;

 if  (nCode == MSGF_DIALOGBOX || nCode == MSGF_MENU)
       if (lpmsg->message == WM_KEYDOWN)
          switch (lpmsg->wParam) {
             case VK_F1:
               PostMessage(hWndMain, WM_COMMAND, IDM_HELP, 0L);
               break;
          }

 DefHookProc(nCode, wParam, lParam, &lpFilterFunc);

 return 0;
}
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ InittheApp - Initialisierung der Applikation (1 mal)               บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

BOOL InittheApp(HINSTANCE hInstance)
{
    HICON hIcon;
    HCURSOR hCursor;
    WNDCLASS wndClass;

    hCursor = LoadCursor((HINSTANCE)NULL,MAKEINTRESOURCE(IDC_ARROW));
    hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_ICON1));

    wndClass.style         = CS_DBLCLKS;
    wndClass.lpfnWndProc   = WndProc;
    wndClass.cbClsExtra    = 0;
    wndClass.cbWndExtra    = 2;
    wndClass.hInstance     = hInstance;
    wndClass.hIcon         = hIcon;
    wndClass.hCursor       = hCursor;
    wndClass.lpszMenuName  = NULL;
    wndClass.lpszClassName = pszAppName;
    wndClass.hbrBackground = 0; // (HBRUSH)COLOR_WINDOW + 1;

    if (!RegisterClass((LPWNDCLASS)&wndClass))
        return FALSE;

    if (!XListBoxRegisterClass(hInstance,(LPSTR)0,0))
       return FALSE;

    return TRUE;
}
/////////////////////////////////////////////////////////////////////
void ReadInProfiles()
/////////////////////////////////////////////////////////////////////
{
  PROFSTRUC * pprofstruc;

  for (pprofstruc = proftab;pprofstruc->name;pprofstruc++)
       *(pprofstruc->wert) = GetPrivateProfileInt(kaptab[pprofstruc->index],
                                                  pprofstruc->name,
                                                  *(pprofstruc->wert),
                                                  pszIniName);
}
/////////////////////////////////////////////////////////////////////
void WriteOutProfiles()
/////////////////////////////////////////////////////////////////////
{
  PROFSTRUC * pprofstruc;
  char  str[128];

  for (pprofstruc = proftab;pprofstruc->name;pprofstruc++)
     {
      if ((pprofstruc->flags == 1)
      || ((pprofstruc->flags == 2) && fSaveOptions))
         {
          _ultoa(*(pprofstruc->wert),str,10);
          WritePrivateProfileString(kaptab[pprofstruc->index],
                                    pprofstruc->name,
                                    str,
                                    pszIniName);
         }
     }
}
/////////////////////////////////////////////////////////////////////
int MsgOutAndExit(int iStringID)
/////////////////////////////////////////////////////////////////////
{
  char str[256];

  LoadString(hInst,iStringID,str,sizeof(str));
  MessageBox(0,str,0,MB_OK);
  return FALSE;
}
/////////////////////////////////////////////////////////////////////
// WinMain - Hauptroutine
/////////////////////////////////////////////////////////////////////
int PASCAL WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpszCmdLine,
                   int cmdShow)
{
    MSG   msg;
    HWND  hWnd;
    HMODULE hKernel;
    SPIFNPTR fpSPI;

    hInst = hInstance;

    memset(dosfn,-1,sizeof(dosfn));

    LoadStringTab(hInst,strloadtab);
    hAccelTable = LoadAccelerators(hInst, MAKEINTRESOURCE(IDA_ACCEL1));
    hMenu = LoadMenu(hInstance,MAKEINTRESOURCE(IDR_MENU1));

    ReadInProfiles();

    if (!hPrevInstance)
        if (!InittheApp(hInstance))
           return MsgOutAndExit(IDS_ERRCLASS);

    if (selecs = AllocDosMemory(TABDIM * sizeof(REGSTRUCT) + 16)) {
//        GlobalFix(LOWORD(selecs));
        lpregstruct = (REGSTRUCT FAR *)MAKELONG(16,LOWORD(selecs));
        lpi1        = (LPINT)MAKELONG(0,LOWORD(selecs));
        *lpi1       = 0;
        *(lpi1+1)   = TABDIM;
        lpzaehler   = (LPWORD)MAKELP(LOWORD(selecs),4);
        *lpzaehler  = 0;
        anzstr      = 0;
    } else
        return MsgOutAndExit(IDS_ERRDOSMEM);

    if (hKernel = GetModuleHandle("USER"))
        if (fpSPI = (SPIFNPTR)GetProcAddress(hKernel,"SYSTEMPARAMETERSINFO"))
            fpSPI(SPI_GETICONTITLELOGFONT,sizeof(LOGFONT),&mylogfont,0);

    tFont = CreateFontIndirect(&mylogfont);

//    GetCopyRightName();

    hWndMain = CreateWindow(
               (LPSTR)pszAppName,
               (LPSTR)pszTitle,
               WS_OVERLAPPEDWINDOW,
               xpos,
               ypos,
               xsize,
               ysize,
               (HWND)NULL,
               (HMENU)hMenu,
               hInstance,
               (LPSTR)NULL
               );

    if (fMonaktiv) {
       ShowWindow(hWndMain,cmdShow);
       UpdateWindow(hWndMain);
    }

    lpFilterFunc = SetWindowsHook(WH_MSGFILTER, (HOOKPROC)FilterFunc);

    while (GetMessage((LPMSG)&msg, NULL, 0, 0))
       {
        hWnd = GetParent(msg.hwnd);
        if (hWnd && (hWnd != hWndMain) && (IsDialogMessage(hWnd,&msg)))
            continue;
        if (!TranslateAccelerator(hWndMain,hAccelTable,(LPMSG)&msg))
           {
            TranslateMessage((LPMSG)&msg);
            DispatchMessage((LPMSG)&msg);
           }
       }

    UnhookWindowsHook(WH_MSGFILTER,(HOOKPROC)FilterFunc);
    FreeDosMemory((HGLOBAL)LOWORD(selecs));
    DeleteObject(tFont);
    WriteOutProfiles();

    return msg.wParam;
}
