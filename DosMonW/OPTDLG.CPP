
#include "string.h"
#include "stdlib.h"
#include "windows.h"
#include "user.h"
#include "userw.h"
#include "dosmonw.h"
#include "dosmonx.h"

extern int regtype;         /* registrierter typ */
extern int fSaveOptions;    /* flag: werden optionen gesichert? */
extern int TimerInterval;   /* refresh intervall */
extern int fCarry;

LRESULT DoCommand(HWND,WPARAM,LPARAM);

/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ OptionDlg - Optionen einstellen                                    บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
BOOL CALLBACK OptionDlg(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam )
{
  static int reqregtype;
  //char str[20];
  int flag,i;
  BOOL rc = FALSE;

  switch (message) {
  case WM_COMMAND:
      switch (wParam) {
      case IDC_TYPEP:
      case IDC_TYPER:
      case IDC_TYPENH:
          switch (HIWORD(lParam)) {
          case BN_CLICKED:
              reqregtype = wParam;
              CheckRadioButton(hDlg,IDC_TYPEP,IDC_TYPENH,reqregtype);
              break;
          }
          break;
      case IDCANCEL:
          PostMessage(hDlg,WM_CLOSE,0,0);
          break;
      case IDOK:
          fCarry = SendDlgItemMessage(hDlg,IDC_CARRY,BM_GETCHECK,0,0);
          fSaveOptions = SendDlgItemMessage(hDlg,IDC_SAVEOPT,BM_GETCHECK,0,0);
          i = GetDlgItemInt(hDlg,IDC_INTERVAL,&flag,FALSE);
          if (flag) {
              if (i != TimerInterval)
                  DoCommand(hDlg,IDM_INTVCHANGE,0);
              TimerInterval = i;
          }
          if (reqregtype != regtype)
              if (!DoCommand(hDlg,IDM_TYPECHANGE,(LPARAM)reqregtype))
                  break;

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
      reqregtype = regtype;
      SendDlgItemMessage(hDlg,reqregtype,BM_SETCHECK,1,0);
      SendDlgItemMessage(hDlg,IDC_CARRY,BM_SETCHECK,fCarry,0);
      SendDlgItemMessage(hDlg,IDC_SAVEOPT,BM_SETCHECK,fSaveOptions,0);
      SendDlgItemMessage(hDlg,IDC_INTERVAL,EM_LIMITTEXT,4,0);
      SetDlgItemInt(hDlg,IDC_INTERVAL,TimerInterval,FALSE);
      ShowWindow(hDlg,SW_SHOWNORMAL);
      rc = TRUE;
      break;
  }
  return rc;
}
