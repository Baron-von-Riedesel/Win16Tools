
#include "string.h"
#include "stdlib.h"
#include "windows.h"
#include "user.h"
#include "userw.h"
#include "dosmonw.h"
#include "dosmonx.h"

extern WORD lzaehler;
extern LPREGSTRUCT lpregstruct;
extern HINSTANCE hInst;

int MyCreateMessage(HWND,UINT);
int searchline(WORD aktzeile);

                                             /* data dictionary */
static DDENTRY strtab[] = {
                  "AX",  0x00,"%04X",DDT_WORD,
                  "BX",  0x02,"%04X",DDT_WORD,
                  "CX",  0x04,"%04X",DDT_WORD,
                  "DX",  0x06,"%04X",DDT_WORD,
                  "SI",  0x08,"%04X",DDT_WORD,
                  "DI",  0x0A,"%04X",DDT_WORD,
                  "BP",  0x0C,"%04X",DDT_WORD,
                  "SP",  0x0E,"%04X",DDT_WORD,
                  "DS",  0x10,"%04X",DDT_WORD,
                  "ES",  0x12,"%04X",DDT_WORD,
                  "SS",  0x14,"%04X",DDT_WORD,
                  "CS",  0x16,"%04X",DDT_WORD,
                  "Flags",0x18,"%04X",DDT_WORD,
                  (PSTR)0};
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ RegisterDlg                                                        บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
BOOL CALLBACK RegisterDlg(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam )
{
  static char szUeb[30] = {0};
  char str[40];
  char str1[20];
  int tabpos[1];
  DDENTRY * pddentry;
  LPREGSTRUCT lpr;
  BOOL rc = FALSE;

  switch (message)
     {
      case WM_COMMAND:
        switch (wParam)
          {
           case IDCANCEL:
             PostMessage(hDlg,WM_CLOSE,0,0);
             break;
           case IDOK:
             PostMessage(hDlg,WM_CLOSE,0,0);
             break;
          }
        rc = TRUE;
        break;
      case WM_CLOSE:
        DestroyWindow(hDlg);
        rc = TRUE;
        break;
      case WM_INITDIALOG:
        lpr = lpregstruct + searchline(LOWORD(lParam));
        if (lpr->user1 == 0xFFFF)
            {
             MyCreateMessage(GetParent(hDlg),IDS_ERRNODOS);
             PostMessage(hDlg,WM_COMMAND,IDOK,0);
             break;
            }
        SendDlgItemMessage(hDlg,IDC_LISTREG,WM_SETREDRAW,0,0);
        SendDlgItemMessage(hDlg,IDC_LISTREG,LB_RESETCONTENT,0,0);
        tabpos[0] = 40;
        SendDlgItemMessage(hDlg,IDC_LISTREG,LB_SETTABSTOPS,1,(LPARAM)(LPSTR)&tabpos);
        pddentry = strtab;
        while (pddentry->name)
           {
            xsprintf(pddentry,str1,(LPSTR)lpr);
            wsprintf(str,"%s\t%s",
                     (LPSTR)pddentry->name,
                     (LPSTR)str1
                    );
            SendDlgItemMessage(hDlg,IDC_LISTREG,LB_ADDSTRING,0,(LONG)(LPSTR)str);
            pddentry++;
           }
        SendDlgItemMessage(hDlg,IDC_LISTREG,WM_SETREDRAW,1,0);
        SendDlgItemMessage(hDlg,IDC_LISTREG,LB_SETCURSEL,0,0);
        if (!szUeb[0])
            LoadString(hInst,IDS_REGUEB,szUeb,sizeof(szUeb));
        wsprintf(str,szUeb,lzaehler - LOWORD(lParam));
        SetWindowText(hDlg,str);
        ShowWindow(hDlg,SW_SHOWNORMAL);
//        rc = TRUE;
        break;
     }
  return rc;
}
