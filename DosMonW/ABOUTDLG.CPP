
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
浜様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様融
� AboutDlg - About Dialog                                            �
藩様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様夕
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
        strcpy(str,"written by Andreas Grech");
        strcat(str,pszDatstr);
        SetDlgItemText(hDlg,IDC_CPYRT,str);
        rc = TRUE;
        break;
     }

  return rc;
}
