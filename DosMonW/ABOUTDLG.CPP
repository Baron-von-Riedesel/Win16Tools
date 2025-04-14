
#include "string.h"
#include "stdlib.h"
#include "windows.h"
#include "user.h"
#include "userw.h"
#include "dosmonw.h"
#include "dosmonx.h"

extern PSTR pszDatstr;
extern HWND hWndMain;

/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ AboutDlg - About Dialog                                            บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
BOOL CALLBACK AboutDlg(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam )
{
  BOOL rc = FALSE;
  char str[60];

  switch (message)
     {
      case WM_COMMAND:
        PostMessage(hDlg,WM_CLOSE,0,0);
        rc = TRUE;
        break;
      case WM_CLOSE:
        EnableWindow(hWndMain,1);
        DestroyWindow(hDlg);
        rc = TRUE;
        break;
      case WM_INITDIALOG:
        EnableWindow(hWndMain,0);
        ShowWindow(hDlg,SW_SHOWNORMAL);
        strcpy(str,"(C) Andreas Grech");
//        strcat(str,GetCopyRightName());
        strcat(str,pszDatstr);
        SetDlgItemText(hDlg,IDC_CPYRT,str);
        rc = TRUE;
        break;
     }

  return rc;
}
