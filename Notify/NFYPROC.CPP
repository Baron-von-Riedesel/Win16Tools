
#include "string.h"
#include "stdlib.h"
#include "io.h"
#include "stdio.h"
#include "windows.h"
#include "toolhelp.h"
#include "user.h"
#include "userw.h"
#include "notify.h"
#include "notifyx.h"

BOOL __loadds CALLBACK NotifyProc(WORD wID,DWORD dwData);

#define GetModuleFromTask(x) (HMODULE)*(HMODULE FAR *)MAKELONG(0x1E,x)
void MakeText(LPNOTSTR,LPSTR,LPSTR);

extern NOTSTRUCT ns;
extern char szLine[];

static NOTSTR notstr;

/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ GetNextPtr()                                                       บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
LPNOTSTR GetNextPtr()
{
 LPNOTSTR  lpr;

  if (ns.fDbgOutput)
      lpr = &notstr;
  else
     {
      lpr = ns.lpns + ns.wNFYIndex++;
      if (ns.wNFYIndex >= TABDIM)
          ns.wNFYIndex = 0;
      ns.wNFYItems++;
     }
  return lpr;
}
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ Output()                                                           บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
void Output(LPNOTSTR lpr)
{
  if (ns.fDbgOutput)
     {
      MakeText(lpr,szLine,"Notify: %04X %-12s %08lX %04X %-8s %s\r\n");
      OutputDebugString(szLine);
     }
}
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ StackTrace                                                         บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
void StackTrace()
{
 STACKTRACEENTRY ste;
 WORD wSS,wCS,wBP,wIP;
 LPNOTSTR  lpr;

  _asm {
        mov ax,[bp+0]
        mov wBP,ax
        mov wSS,ss
        mov ax,offset NotifyProc
        mov wIP,ax
        mov wCS,cs
       };

  ste.dwSize = sizeof(STACKTRACEENTRY);
  if (StackTraceCSIPFirst(&ste,wSS,wCS,wIP,wBP))
     while (StackTraceNext(&ste))
        {
         lpr = GetNextPtr();
         lpr->wID = 14;
         lpr->dwData = MAKELONG(ste.wIP,ste.wCS);
         lpr->hTask  = 0;
         lpr->szTask[0] = 0;
         Output(lpr);
        }
}
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ NotifyProc - Monitor (nur protected mode)                          บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
BOOL __loadds CALLBACK NotifyProc(WORD wID,DWORD dwData)
{
 HMODULE   hModule;
 LPNOTSTR  lpr;
 LPSTR     lpStr;
 LPVOID    lpvoid;

 if (wID > 13)
     return 0;

 if (ns.fAktTab[wID] & 1)
    {
     lpr = GetNextPtr();
     lpr->dwData = dwData;
     lpr->wID    = wID;
     lpr->hTask  = GetCurrentTask();
     lpr->szTask[0] = 0;
     if (lpr->hTask)
        {
         if (hModule = GetModuleFromTask(lpr->hTask))
             GetModuleName(hModule,lpr->szTask,sizeof(lpr->szTask));
        }
     lpvoid  = (LPVOID)dwData;
     switch (lpr->wID)
        {
         case NFY_LOADSEG:        /* 01 */
            lpr->w1 = ((NFYLOADSEG FAR *)lpvoid)->wSelector;
            _fstrncpy(lpr->szModule,
                      ((NFYLOADSEG FAR *)lpvoid)->lpstrModuleName,
                      sizeof(lpr->szModule));
            lpr->szModule[sizeof(lpr->szModule)-1] = 0;
            lpr->dwData = ((NFYLOADSEG FAR *)lpvoid)->wSegNum;
            break;
         case NFY_DELMODULE:      /* 06 */
            if (lpr->hModule = (HMODULE)LOWORD(lpr->dwData))
                GetModuleName(lpr->hModule,lpr->szModule,sizeof(lpr->szModule));
            break;
         case NFY_STARTDLL:       /* 03 */
            if (lpr->hModule = ((NFYSTARTDLL FAR *)lpvoid)->hModule)
                GetModuleName(lpr->hModule,lpr->szModule,sizeof(lpr->szModule));
            break;
         case NFY_FREESEG:        /* 02 */
//////        if (lpr->hModule = GetExePtr((HGLOBAL)LOWORD(dwData)))
//////            GetModuleName(lpr->hModule,lpr->szModule,sizeof(lpr->szModule));
            break;
         case NFY_LOGPARAMERROR:  /* 13 */
            lpr->dwData = (DWORD)((NFYLOGPARAMERROR FAR *)lpvoid)->lpfnErrorAddr;
            lpr->w1 = ((NFYLOGPARAMERROR FAR *)lpvoid)->wErrCode;
            break;
         case NFY_LOGERROR:       /* 12 */
            lpr->w1 = (DWORD)((NFYLOGERROR FAR *)lpvoid)->wErrCode;
            break;
         case NFY_OUTSTR:         /* 11 */
            if (ns.lpStrings)
               {
                lpr->dwData = (DWORD)ns.lpStrings;
                for (lpStr = (LPSTR)dwData;*lpStr;lpStr++)
                    if ((*lpStr != 0x0D) && (*lpStr != 0x0A))
                        *ns.lpStrings++ = *lpStr;
                *ns.lpStrings++ = 0;
               }
            break;
         case NFY_UNKNOWN:        /* 00 */
         case NFY_STARTTASK:      /* 04 */
         case NFY_EXITTASK:       /* 05 */
         case NFY_RIP:            /* 07 */
         case NFY_TASKIN:         /* 08 */
         case NFY_TASKOUT:        /* 09 */
         case NFY_INCHAR:         /* 10 */
             break;
        }  // end switch (lpr->wID)

     Output(lpr);
    }

 if (ns.fAktTab[wID] & 4)
     StackTrace();

 if (ns.fAktTab[wID] & 2)
     DebugBreak();

 return 0;
}
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ InterruptProc - wegen Bug in Toolhelp()                            บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
void EXPORTED CALLBACK InterruptProc(void)
{
}
