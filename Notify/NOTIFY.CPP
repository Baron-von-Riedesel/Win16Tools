
#include "string.h"
#include "stdlib.h"
#include "io.h"
#include "sys\stat.h"
#include "fcntl.h"
#include "stdio.h"
#include "windows.h"
#include "toolhelp.h"
#include "commdlg.h"
#include "user.h"
#include "userw.h"
#include "notify.h"
#include "notifyx.h"

#define ANZTAB 6
#define MAXPATH 260

BOOL CALLBACK AboutDlg    (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK RegisterDlg (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK OptionDlg   (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK NotifyProc  (WORD, DWORD);
void CALLBACK InterruptProc(void);

typedef BOOL (CALLBACK *SPIFNPTR)(UINT,UINT,VOID FAR *,UINT);

LRESULT EXPORTED CALLBACK WndProc  (HWND, UINT, WPARAM, LPARAM);
WORD EXPORTED CALLBACK TimerProc(HWND ,WORD    , int,  DWORD);

long DoCommand   (HWND, UINT, WPARAM, LPARAM);
BOOL InittheApp  (HINSTANCE);

// Globale Variable

HWND      hWndMain   = 0;         // hWnd main window
HWND      hWndChild  = 0;         // hWnd child window
HWND      hWndOption = 0;         // hWnd OptionDlg
HINSTANCE hInst      = 0;         // handle instance
HMENU     hMenu      = 0;         // handle menu (menu wird modifiziert)
HFONT     hFont      = 0;         // handle des kreierten fonts
HTASK     hTask      = 0;
HANDLE    hLibCtl3d  = 0;
LPFNNOTIFYCALLBACK  lpNotifyCallback;

NOTSTRUCT ns = {0x41,    // intnr
                2000,    // timerintervall
                0,0,0,0,0, // allgemeine Flags
                0,       // hGMem
                0,       // hStrings
                0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0,0,0,0
               };

                                 // pointer auf resource strings
PSTR pszAppName,pszTitle;
PSTR pszIniName;
PSTR pszInactiv;
PSTR pszWinCap;
char szTitle[80];
int  iCharh;                       // zeichenhoehe
char szLine[128];

#include "letab.h"
#include "leptab.h"

// statische Variable
                                 // tabs
static int tabpos[ANZTAB];
static int tabdiffs[ANZTAB+1] = {40, 150, 80, 150, 250, 9000, 0 };

static char * lpFilters = {"Alle Dateien\000*.*\000\000\000"};

static char * szStrTab[15] = {
     "Unknown",
     "LoadSeg",
     "FreeSeg",
     "StartDll",
     "StartTask",
     "ExitTask",
     "DelModule",
     "RIP",
     "TaskIn",
     "TaskOut",
     "InChar",
     "OutStr",
     "LogError",
     "LogParamError",
     "Stacktrace"
    };

// tabelle fuer string load
static STRLOADENTRY strloadtab[] = {
                       &pszAppName, IDS_APPNAME,
                       &pszTitle,   IDS_TITLE,
                       &pszIniName, IDS_ININAME,
                       &pszWinCap,  IDS_WINCAP,
                       (PSTR *)(0)};

/* profile load tabelle */
static PROFSTRUC proftab[] = {
                       2,0,"TimerInterval",&ns.TimerInterval,
                       1,0,"SaveOptions",&ns.fSaveOptions,
                       2,0,"DbgOutput",&ns.fDbgOutput,
                       2,0,"IntReg",&ns.fIntReg,
                       0,0,(PSTR)0,NULL
                      };
static char * kaptab[] = {"parms"};

static LOGFONT lf = {
        8,                           // height
        0,                           // width
        0,                           // escapement
        0,                           // orientation
        700,                         // weight
        FALSE,                       // italic
        FALSE,                       // underline
        FALSE,                       // strikeout
        ANSI_CHARSET,                // charset
        OUT_DEFAULT_PRECIS,          // przision(const)
        CLIP_DEFAULT_PRECIS,         // przision(const)
        DEFAULT_QUALITY,             //
        0,                           // pitch & family
        "Helv"};                     // Fontname

static BOOL (FAR PASCAL *THProc)(void) = 0;
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ GetSaveArea - Wenn Ausgabe auf Window                              บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
BOOL GetSaveArea()
{
  if (ns.hGMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                             TABDIM * sizeof(NOTSTR)))
     {
      ns.lpns          = (NOTSTR FAR *)GlobalLock(ns.hGMem);
      ns.wNFYIndex     = 0;
      ns.wNFYItems     = 0;
      ns.wActItem      = 0;
      ns.wNumItems     = 0;
      ns.anzstr        = 0;
      ns.hStrings      = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                     0x10000);
      ns.lpStrings     = (LPSTR)GlobalLock(ns.hStrings);
      return TRUE;
     }
  else
      return FALSE;
}
BOOL OutputModeChanged()
{
 char str[80];

 if (!ns.fDbgOutput)
     if (!ns.hGMem)
         if (!GetSaveArea())
            {
             LoadString(hInst,IDS_ERRMEM,str,sizeof(str));
             MessageBox(hWndMain,str,0,MB_OK);
             ns.fDbgOutput = TRUE;
             return FALSE;
            }
 return TRUE;
}
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ ActivateTimer - Timer ein/ausschalten                              บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
void ActivateTimer(WORD wMode)
{
 static fTimer = 0;
 if ((wMode) && (!fTimer))
    {
     if (!ns.fDbgOutput)
        {
         fTimer = TRUE;
         SetTimer(0,1,ns.TimerInterval,(TIMERPROC)TimerProc);
        }
    }
 else
 if ((!wMode) && (fTimer))
    {
     fTimer = FALSE;
     KillTimer(hWndMain,1);
    }
}
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ auszugebenden Text einer Zeile aufbereiten                         บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
void MakeText(LPNOTSTR lpr,LPSTR lpDest,LPSTR lpFStr)
{
  char str1[100];
  LESTRUCT * pLE;
  LEPSTRUCT * pLEP;
  HMODULE hModule;
  LPSTR lpStr;
  char str2[80];
  char str3[80];
  FARPROC fpEntry;
  LPSTR lpszStr;

  str1[0] = 0;
  str2[0] = 0;
  lpszStr = (LPSTR)0;
  lpStr = str1;

  switch (lpr->wID)
     {
      case NFY_LOADSEG:        /* 01 */
         wsprintf(str1,"Sel=%04X (%s.%u)",lpr->w1,(LPSTR)lpr->szModule,LOWORD(lpr->dwData));
         break;
      case NFY_DELMODULE:      /* 06 */
      case NFY_STARTDLL:       /* 03 */
         wsprintf(str1,"Modul=%04X,%s",lpr->hModule,(LPSTR)lpr->szModule);
         break;
      case NFY_STARTTASK:      /* 04 */
         wsprintf(str1,"CS:IP=%04X:%04X",HIWORD(lpr->dwData),LOWORD(lpr->dwData));
         break;
      case NFY_FREESEG:        /* 02 */
//         wsprintf(str1,"Modul=%04X,%s",lpr->hModule,(LPSTR)lpr->szModule);
         break;
      case NFY_LOGPARAMERROR:  /* 13 */
         if (hModule = GetExePtr((HGLOBAL)HIWORD(lpr->dwData)))
             GetModuleName(hModule,str2,sizeof(str2));
         if (fpEntry = GetPrevEntryAddress(hModule,(FARPROC)lpr->dwData))
             GetSymbolicNameEx(fpEntry,str3);
         else
             str3[0] = 0;
         for (pLEP = leptab;pLEP->wErrCode;pLEP++)
             if (pLEP->wErrCode == (lpr->w1))
                 break;
         wsprintf(str1,"CS:IP=%4X:%04X,ErrCode=%X\t%s.%s,%s",
                  HIWORD(lpr->dwData),
                  LOWORD(lpr->dwData),
                  lpr->w1,
                  (LPSTR)str2,
                  (LPSTR)str3,
                  (LPSTR)pLEP->pStr);
         break;
      case NFY_LOGERROR:       /* 12 */
         for (pLE = letab;pLE->wErrCode;pLE++)
             if (pLE->wErrCode == (lpr->w1 & 0x0FFF))
                 break;
         wsprintf(str1,"ErrCode=%X\t%s",lpr->w1,(LPSTR)pLE->pStr);
         break;
      case 14:
         if (hModule = GetExePtr((HINSTANCE)HIWORD(lpr->dwData)))
             GetModuleName(hModule,str2,sizeof(str2));
         wsprintf(str1,"CS:IP=%X:%04X\t%s",
                  HIWORD(lpr->dwData),
                  LOWORD(lpr->dwData),
                  (LPSTR)str2
                  );
         break;
      case NFY_OUTSTR:         /* 11 */
         lpStr = (LPSTR)lpr->dwData;
         break;
      case NFY_UNKNOWN:        /* 00 */
      case NFY_EXITTASK:       /* 05 */
      case NFY_RIP:            /* 07 */
      case NFY_TASKIN:         /* 08 */
      case NFY_TASKOUT:        /* 09 */
      case NFY_INCHAR:         /* 10 */
         break;
      default:
         lpszStr = "???";
         break;
     }
  if (!lpszStr)
      lpszStr = szStrTab[lpr->wID];

  wsprintf(lpDest,
           lpFStr,
           lpr->wID,
           lpszStr,
           lpr->dwData,
           lpr->hTask,
           (LPSTR)lpr->szTask,
           (LPSTR)lpStr
          );
}
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ TimerProc - nur bei Ausgabe in Window                              บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
WORD EXPORTED CALLBACK TimerProc(HWND hWnd,WORD wMsg,int idEvent,DWORD dwtime)
{
 int diff;
 int diffstr;
 WORD index;
 WORD selec;
 WORD anzzeilen;

                     // anzahl der inzwischen neu hinzugekommenen eintraege
 diff = ns.wNFYItems;

 if (diff)
   {
    if (ns.anzstr < TABDIM)
       {
        if ((ns.anzstr + diff) > TABDIM)
           {
            diffstr = TABDIM - ns.anzstr;
            ns.anzstr = TABDIM;
           }
        else
           {
            ns.anzstr = ns.anzstr + diff;
            diffstr = diff;
           }
        SendMessage(hWndChild,XLB_ADDMULTSTRINGS,diffstr,0);
       }

    index = (WORD)SendMessage(hWndChild,LB_GETTOPINDEX,0,0);
    selec = (WORD)SendMessage(hWndChild,LB_GETCURSEL,0,0);
    anzzeilen = (WORD)SendMessage(hWndChild,XLB_GETLINES,0,0);
                                                         /* nonstandard lParam */
    if ((index + diff) >= (TABDIM - anzzeilen))
       {
        SendMessage(hWndChild,XLB_CHANGECURPOS,TABDIM - anzzeilen,0);
        InvalidateRect(hWndChild,0,0);
       }
    else
        SendMessage(hWndChild,XLB_CHANGECURPOS,index + diff,0);

    if (!index)
        SendMessage(hWndChild,WM_VSCROLL,SB_TOP,0);

    ns.wNFYItems = ns.wNFYItems - diff;

    ns.wActItem = ns.wActItem + diff;
    if (ns.wActItem >= TABDIM)
        ns.wActItem = ns.wActItem - TABDIM;

    ns.wNumItems = ns.wNumItems + diff;
    if (ns.wNumItems > TABDIM)
        ns.wNumItems = TABDIM;
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
int RegisterIt(HWND hWnd, BOOL wMode)
{
 int rc = FALSE;
 int nfMode = NF_NORMAL;

 if (wMode) {
     if (ns.fMonActive)
         RegisterIt(hWnd,0);

     if (ns.fAktTab[NFY_RIP])
        nfMode = nfMode | NF_RIP;
     if (ns.fAktTab[NFY_TASKIN] || ns.fAktTab[NFY_TASKOUT])
        nfMode = nfMode | NF_TASKSWITCH;
     if (rc = NotifyRegister(hTask,lpNotifyCallback, nfMode)) {
         ns.fMonActive = TRUE;
         if (ns.fIntReg) {
             InterruptRegister(hTask,(FARPROC)InterruptProc);
             ns.fIntRegistered = TRUE;
         }
         SetWindowText(hWnd,szTitle);
     }
 } else {
     if (ns.fMonActive)
        {
         if (rc = NotifyUnRegister(hTask))
            ns.fMonActive = FALSE;
        }
     if (ns.fIntRegistered)
        {
         if (InterruptUnRegister(hTask))
             ns.fIntRegistered = FALSE;
        }
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
  int i;
  LPNOTSTR lpr;
  //LPSTR lpszStr;

                                  // i -> Index 0 in tabelle
  i  = ns.wActItem - 1;
  i = i - lpdistruct->itemID;
  if (i < 0)
      i = i+TABDIM;

  lpr = ns.lpns + i;

  MakeText(lpr,szLine,"%04X\t%s\t%08lX\t%04X %s\t%s\t\t");

  if (lpdistruct->itemState & ODS_SELECTED)
     {
      SetBkColor(lpdistruct->hDC,GetSysColor(COLOR_HIGHLIGHT));
      SetTextColor(lpdistruct->hDC,GetSysColor(COLOR_HIGHLIGHTTEXT));
     }

  TabbedTextOut(lpdistruct->hDC,lpdistruct->rcItem.left,lpdistruct->rcItem.top,szLine,strlen(szLine),ANZTAB,tabpos,0);

  if (lpdistruct->itemState & ODS_FOCUS)
      DrawFocusRect(lpdistruct->hDC,&lpdistruct->rcItem);

  if (lpdistruct->itemState & ODS_SELECTED)
     {
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

  if (prect->top < iCharh)
     {
      SelectObject(hdc,hFont);
      SetBkColor(hdc,GetSysColor(COLOR_WINDOW));
      SetTextColor(hdc,GetSysColor(COLOR_MENUTEXT));
      TabbedTextOut(hdc,0,0,pszWinCap,strlen(pszWinCap),ANZTAB,tabpos,0);
      MoveTo(hdc,0,iCharh-1);
      LineTo(hdc,prect->right,iCharh-1);
     }
}
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ WM_COMMAND ID_SAVEAS verarbeiten                     บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
void HandleSaveas(HWND hWnd)
{
  char   str[MAXPATH] = "";
  OPENFILENAME ofn;
  LPNOTSTR lpr;
  int i,j;
  int hFile;
  int rc;

  memset(&ofn,0,sizeof(OPENFILENAME));

  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner   = hWnd;
  ofn.hInstance   = hInst;
  ofn.lpstrFilter = lpFilters;
  ofn.lpstrCustomFilter = 0;
  ofn.nMaxCustFilter = 0;
  ofn.nFilterIndex = 0;
  ofn.lpstrFile = str;
  ofn.nMaxFile  = sizeof(str);
  ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
  if (rc = GetSaveFileName(&ofn))
     {
      if ((hFile = _open(str,
                        _O_WRONLY | _O_BINARY | _O_CREAT | _O_TRUNC,
                        _S_IREAD | _S_IWRITE)) != -1)


      j = ns.wActItem - ns.wNumItems;
      if (j < 0)
          j = j + TABDIM;

      for (i=0 ;i<ns.wNumItems;i++,j++)
        {
         if (j>=TABDIM)
             j = 0;
         lpr = ns.lpns + j;
         MakeText(lpr,str,"%04X\t%s\t%08lX\t%04X %s\t%s\r\n");
         _write(hFile,str,strlen(str));
        }
      _close(hFile);
     }
}
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ DoCommand - WM_COMMAND verarbeiten                                 บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
LRESULT DoCommand(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
  int len;
  PSTR pstr;
  char str[80];

  switch (wParam)
    {
    case ID_MAINLB:
      switch (HIWORD(lParam))
        {
         case LBN_DBLCLK:
            break;
        }
      break;
    case ID_SAVEAS:
      HandleSaveas(hWnd);
      break;
    case ID_EXIT:
      PostMessage(hWnd,WM_CLOSE,0,0);
      break;
    case ID_HELP:
      len = GetModuleFileName(hInst,str,sizeof(str));
      pstr = str + len - 3;
      strcpy(pstr,"hlp");
      WinHelp(hWnd,str,HELP_INDEX,0);
      break;
    case ID_CLEAR:
      if (ns.hGMem)
         {
          ns.wNFYIndex  = 0;
          ns.wNFYItems  = 0;
          ns.wActItem   = 0;
          ns.wNumItems  = 0;
          ns.anzstr     = 0;
          SendMessage(hWndChild,LB_RESETCONTENT,0,0);
         }
      break;
    case ID_ABOUT:
      CreateDialog(hInst,MAKEINTRESOURCE(IDD_ABOUTBOX),hWnd,AboutDlg);
      break;
    case ID_OPTION:
      if (hWndOption)
         ShowWindow(hWndOption,SW_RESTORE);
      else
         hWndOption = CreateDialog(hInst,
                                   MAKEINTRESOURCE(IDD_OPTIONBOX),
                                   hWnd,
                                   OptionDlg);
      break;
    default:
      break;
    }
 return 0;
}

void WriteState(HWND hWnd)
//////////////////////////
{
  char str[80];
  PROFSTRUC * pprofstruc;
  WINDOWPLACEMENT wpl;

   for (pprofstruc = proftab;pprofstruc->name;pprofstruc++)
      {
       if ((pprofstruc->flags == 1)
       || ((pprofstruc->flags == 2) && ns.fSaveOptions))
          {
           wsprintf(str,"%u",*(pprofstruc->wert));
           WritePrivateProfileString(kaptab[pprofstruc->index],
                                     pprofstruc->name,
                                     str,
                                     pszIniName);
          }
      }
   wvsprintf(str,"%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u",ns.fAktTab);
   WritePrivateProfileString(kaptab[0],"Actions",str,pszIniName);

   wpl.length = sizeof(wpl);
   GetWindowPlacement(hWnd,&wpl);
   wsprintf(str,
            "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
            wpl.flags,
            wpl.showCmd,
            wpl.ptMinPosition.x,
            wpl.ptMinPosition.y,
            wpl.ptMaxPosition.x,
            wpl.ptMaxPosition.y,
            wpl.rcNormalPosition.left,
            wpl.rcNormalPosition.top,
            wpl.rcNormalPosition.right,
            wpl.rcNormalPosition.bottom
           );
   WritePrivateProfileString(kaptab[0],"WndState",str,pszIniName);
}
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ WndProc - Windowroutine                                            บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
LRESULT EXPORTED CALLBACK WndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
  PAINTSTRUCT  ps;
  int rflag;
  PSTR pstr;
  PINT pint1,pint2;
  char str[80];
  //RECT rect;
  int len;
  long rc;

    rc = FALSE;
    switch (message)
      {
        case WM_CREATE:

            /* Make/FreeProcInstance() not needed since we use __loadds */
            //lpNotifyCallback = (LPFNNOTIFYCALLBACK)MakeProcInstance((FARPROC)NotifyProc,hInst);
            lpNotifyCallback = NotifyProc;
            iCharh = GetCharHeight(hWnd,hFont);
            ActivateTimer(1);
            rflag = RegisterIt(hWnd,1);
            pint1 = tabdiffs;
            pint2 = tabpos;
            *pint2++ = *pint1++;
            while (*pint1)
                *pint2++ = *(pint2-1) + *pint1++;

            if (rflag) // registrierung ok?
               {
                hWndChild = CreateWindow("xlistbox",
                                         "",
                                         WS_CHILD | WS_VISIBLE | LBS_OWNERDRAWFIXED | LBS_WANTKEYBOARDINPUT,
                                         0,
                                         iCharh,
                                         100,
                                         100,
                                         hWnd,
                                         (HMENU)ID_MAINLB,
                                         hInst,
                                         0
                                        );
                SendMessage(hWndChild,WM_SETFONT,(WPARAM)hFont,0);
               }
            else
               {
                LoadString(hInst,IDS_ERRREG,str,sizeof(str));
                MessageBox(0,str,0,MB_OK);
                rc = -1;
               }

            break;
        case WM_DESTROY:
            if (ns.hGMem)
                GlobalFree(ns.hGMem);
            DeleteObject(hFont);
            WriteState(hWnd);
            PostQuitMessage(0);
            break;
        case WM_ENDSESSION:
            if (wParam)
               {
                WriteState(hWnd);
                RegisterIt(hWnd,0);
               }
            break;
        case WM_CLOSE:
            RegisterIt(hWnd,0);
            //FreeProcInstance((FARPROC)lpNotifyCallback);
            ActivateTimer(0);
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
            break;
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
            rc = DoCommand(hWnd,message,wParam,lParam);
            break;
        case WM_SIZE:                                       /* fenstergroesse merken */
            if (HIWORD(lParam) % iCharh)
                AdjustClientSize(hWnd,0,(HIWORD(lParam) / iCharh) * iCharh,1);
            else
                MoveWindow(hWndChild,0,iCharh,LOWORD(lParam),HIWORD(lParam) - iCharh,TRUE);
            break;
        default:
            rc = DefWindowProc(hWnd, message, wParam, lParam);
            break;
    }
    return rc;
}
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ InitFirstInstance - Initialisierung 1. (und einzige) Instanz       บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

BOOL InitFirstInstance(HINSTANCE hInstance)
{
    HICON hIcon;
    HCURSOR hCursor;
    WNDCLASS wndClass;

    hCursor = LoadCursor(NULL, IDC_ARROW);
    hIcon   = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_ICON1));

    wndClass.style         = CS_DBLCLKS;
    wndClass.lpfnWndProc   = (WNDPROC)WndProc;
    wndClass.cbClsExtra    = 0;
    wndClass.cbWndExtra    = 2;
    wndClass.hInstance     = hInstance;
    wndClass.hIcon         = hIcon;
    wndClass.hCursor       = hCursor;
    wndClass.lpszMenuName  = NULL;
    wndClass.lpszClassName = pszAppName;
    wndClass.hbrBackground = (HBRUSH)0;

    if (!RegisterClass((LPWNDCLASS)&wndClass))
        return FALSE;

    if (!XListBoxRegisterClass(hInstance,(LPSTR)0,0))
       return FALSE;

    return TRUE;
}
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ InitInstance - Initialisierung Instanz                             บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
BOOL InitInstance(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpszCmdLine,int cmdShow)
{
  WINDOWPLACEMENT wpl;
  //int   fHandle;
  //int   i;
  char  str[128];
  HMODULE hKernel;
  PROFSTRUC * pprofstruc;
  SPIFNPTR fpSPI;

  hInst = hInstance;
  hTask = GetTaskFromInstance(hInst);

  LoadStringTab(hInst,strloadtab);
  hMenu = LoadMenu(hInstance,MAKEINTRESOURCE(IDM_MENU1));

  for (pprofstruc = proftab;pprofstruc->name;pprofstruc++)
       *(pprofstruc->wert) = GetPrivateProfileInt(kaptab[pprofstruc->index],
                                                  pprofstruc->name,
                                                  *(pprofstruc->wert),
                                                  pszIniName);

  GetPrivateProfileString(kaptab[0],"Actions",str,str,sizeof(str),pszIniName);

// Vorsicht: Visual C 1.5 mit Option -G3 erzeugt hier fehlerhaften Code

  sscanf(str,"%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u",
         &ns.fAktTab[0],
         &ns.fAktTab[1],
         &ns.fAktTab[2],
         &ns.fAktTab[3],
         &ns.fAktTab[4],
         &ns.fAktTab[5],
         &ns.fAktTab[6],
         &ns.fAktTab[7],
         &ns.fAktTab[8],
         &ns.fAktTab[9],
         &ns.fAktTab[10],
         &ns.fAktTab[11],
         &ns.fAktTab[12],
         &ns.fAktTab[13]);

  if (!hPrevInstance)
      if (!InitFirstInstance(hInstance))
         {
          LoadString(hInst,IDS_ERRCLASS,str,sizeof(str));
          MessageBox(0,str,0,MB_OK);
          return FALSE;
         }

  if (!OutputModeChanged())
      return FALSE;

  if (hKernel = GetModuleHandle("USER"))
      if (fpSPI = (SPIFNPTR)GetProcAddress(hKernel,"SYSTEMPARAMETERSINFO"))
          fpSPI(SPI_GETICONTITLELOGFONT,sizeof(LOGFONT),&lf,0);

  hFont = CreateFontIndirect(&lf);

  //GetCopyRightName();

  wsprintf(szTitle,pszTitle,ns.intnr);

  hWndMain = CreateWindow(
             (LPSTR)pszAppName,
             (LPSTR)str,
             WS_OVERLAPPEDWINDOW,
             CW_USEDEFAULT,
             CW_USEDEFAULT,
             CW_USEDEFAULT,
             CW_USEDEFAULT,
             (HWND)NULL,
             hMenu,
             hInstance,
             (LPSTR)NULL
             );

  if (!hWndMain)
      return FALSE;

  wpl.length = sizeof(wpl);
  GetPrivateProfileString(kaptab[0],"WndState",str,str,sizeof(str),pszIniName);
  if (10 == sscanf(str,
         "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
         &wpl.flags,
         &wpl.showCmd,
         &wpl.ptMinPosition.x,
         &wpl.ptMinPosition.y,
         &wpl.ptMaxPosition.x,
         &wpl.ptMaxPosition.y,
         &wpl.rcNormalPosition.left,
         &wpl.rcNormalPosition.top,
         &wpl.rcNormalPosition.right,
         &wpl.rcNormalPosition.bottom
        ))
      SetWindowPlacement(hWndMain,&wpl);
  else
      ShowWindow(hWndMain,cmdShow);

  return TRUE;
}
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ WinMain - Hauptroutine                                             บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
int PASCAL WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpszCmdLine,int cmdShow)
{
    MSG   msg;
    HWND  hWnd;

    if (hPrevInstance) {
        GetInstanceData(hPrevInstance,(PBYTE)&hWndMain,sizeof(hWndMain));
        PostMessage(hWndMain,WM_SYSCOMMAND,SC_RESTORE,0);
        return 1;
       }

    if (!InitInstance(hInstance,hPrevInstance,lpszCmdLine,cmdShow))
        return 1;

    hLibCtl3d = TryCtl3dRegister(hInstance,(HINSTANCE)0);

    while (GetMessage((LPMSG)&msg, NULL, 0, 0))
       {
        if (msg.hwnd)
           hWnd = GetParent(msg.hwnd);
        else
           hWnd = 0;
        if (hWnd && (hWnd != hWndMain) && (IsDialogMessage(hWnd,&msg)))
            continue;
        TranslateMessage((LPMSG)&msg);
        DispatchMessage((LPMSG)&msg);
       }

    if (hLibCtl3d)
        TryCtl3dRegister(hInstance,(HINSTANCE)hLibCtl3d);

    return msg.wParam;
}
