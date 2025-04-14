
#include "string.h"
#include "stdlib.h"
#include "windows.h"
#include "toolhelp.h"
#include "user.h"
#include "userw.h"
#include "notify.h"
#include "notifyx.h"

void ActivateTimer(WORD);
BOOL OutputModeChanged();
int  RegisterIt(HWND,BOOL);

extern NOTSTRUCT ns;
extern HWND hWndOption;

/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ OptionDlg - Optionen einstellen                                    บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
BOOL EXPORTED CALLBACK OptionDlg(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam )
{
  //char str[20];
  HWND hWnd;
  int flag,i;
  BOOL rc;

  switch (message)
     {
//      case WM_SYSCOLORCHANGE:
//             Ctl3dColorChange();
//             break;
      case WM_COMMAND:
        switch (wParam)
          {
           case IDCANCEL:
             PostMessage(hDlg,WM_CLOSE,0,0);
             break;
           case IDOK:
             ActivateTimer(0);

             for (i=0;i<14;i++)
                {
                 ns.fAktTab[i] = ns.fAktTab[i] & 0xFFF8;

                 if (SendDlgItemMessage(hDlg,ID_UNKNOWN+i,BM_GETCHECK,0,0))
                     ns.fAktTab[i] = ns.fAktTab[i] | 1;
                 if (SendDlgItemMessage(hDlg, ID_UNKNOWN+i+20,BM_GETCHECK,0,0))
                     ns.fAktTab[i] = ns.fAktTab[i] | 2;
                 if (hWnd = GetDlgItem(hDlg,ID_UNKNOWN+i+40))
                     if (SendMessage(hWnd,BM_GETCHECK,0,0))
                         ns.fAktTab[i] = ns.fAktTab[i] | 4;
                }
             ns.fDbgOutput   = (BOOL)SendDlgItemMessage(hDlg,ID_DBGOPT, BM_GETCHECK,0,0);
             ns.fIntReg      = (BOOL)SendDlgItemMessage(hDlg,ID_INTREG, BM_GETCHECK,0,0);
             ns.fSaveOptions = (BOOL)SendDlgItemMessage(hDlg,ID_SAVEOPT,BM_GETCHECK,0,0);
             i = GetDlgItemInt(hDlg,ID_INTERVAL,&flag,FALSE);
             if (flag)
                ns.TimerInterval = i;

             ActivateTimer(1);
             OutputModeChanged();
             RegisterIt(hDlg,ns.fMonActive);
             PostMessage(hDlg,WM_CLOSE,0,0);
             break;
          }
        rc = TRUE;
        break;
      case WM_DESTROY:
        hWndOption = 0;
        break;
      case WM_CLOSE:
        DestroyWindow(hDlg);
        rc = TRUE;
        break;
      case WM_INITDIALOG:
        for (i = 0;i < 14;i++)
           {
            SendDlgItemMessage(hDlg,
                               ID_UNKNOWN+i,
                               BM_SETCHECK,
                               ns.fAktTab[i] & 1,0);
            SendDlgItemMessage(hDlg,
                               ID_UNKNOWN+i+20,
                               BM_SETCHECK,
                               ns.fAktTab[i] & 2,0);
            if (hWnd = GetDlgItem(hDlg,ID_UNKNOWN+i+40))
                SendDlgItemMessage(hDlg,
                                   ID_UNKNOWN+i+40,
                                   BM_SETCHECK,
                                   ns.fAktTab[i] & 4,0);
           }
        SendDlgItemMessage(hDlg,ID_SAVEOPT,  BM_SETCHECK,ns.fSaveOptions,0);
        SendDlgItemMessage(hDlg,ID_DBGOPT,   BM_SETCHECK,ns.fDbgOutput,0);
        SendDlgItemMessage(hDlg,ID_INTREG,   BM_SETCHECK,ns.fIntReg,0);
        SendDlgItemMessage(hDlg,ID_INTERVAL,EM_LIMITTEXT,4,0);
        SetDlgItemInt(hDlg,ID_INTERVAL,ns.TimerInterval,FALSE);

        ShowWindow(hDlg,SW_SHOWNORMAL);
        rc = TRUE;
        break;
      default:
        rc = FALSE;
        break;
     }
  return rc;
}
