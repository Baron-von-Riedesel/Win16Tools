
#include "string.h"
#include "stdlib.h"
#include "windows.h"
#include "toolhelp.h"
#include "user.h"
#include "userw.h"
#include "notify.h"
#include "notifyx.h"

/*
浜様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様融
� AboutDlg - About Dialog                                            �
藩様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様夕
*/
BOOL EXPORTED CALLBACK AboutDlg(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam )
{
  BOOL rc;
  char str[60];

  switch (message)
     {
      case WM_INITDIALOG:
        EnableWindow(hWndMain,0);
        ShowWindow(hDlg,SW_SHOWNORMAL);
        strcpy(str,"(C) Andreas Grech 1993");
        SetDlgItemText(hDlg,ID_CPYRT,str);
        rc = TRUE;
        break;
      case WM_CLOSE:
        EnableWindow(hWndMain,1);
        DestroyWindow(hDlg);
        rc = TRUE;
        break;
      case WM_COMMAND:
        PostMessage(hDlg,WM_CLOSE,0,0);
        rc = TRUE;
        break;
      default:
        rc = FALSE;
        break;
     }

  return rc;
}
