
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "windows.h"
#include "user.h"
#include "userw.h"
#include "dos.h"
#include "dosmonw.h"
#include "dosmonx.h"

int xFilepos = 50;
int yFilepos = 50;

LPVOID GetKernelPSP()
{
 LPVOID lpvoid;
 _asm {
       call GetCurrentPDB
       xor  ax,ax
       mov  word ptr lpvoid+0,ax
       mov  word ptr lpvoid+2,dx
      }
 return lpvoid;
}
/*
ษออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ FileDlg - File Dialog                                บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/
BOOL CALLBACK FileDlg(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam )
{
 char  str[80];
 char  str1[9];
 char  str2[4];
 char  str3[40];
 char  * pstr;
 char  mchar;
 int   y,x,i,j;
 int tabpos[6];
 HWND hWnd;
 DWORD dw[2];
 LPWORD lpWord;
 SFTENTRY sftentry;
 LPBYTE lpfindex;
 LPWORD lpword;
 HGLOBAL hMem;
 long memsize,memoffs;
 int zflag;
 BOOL rc = FALSE;

 switch (message)
    {
    case WM_COMMAND:
      switch (wParam)
         {
         case IDCANCEL:
         case IDOK:
              PostMessage(hDlg,WM_CLOSE,0,0);
              break;
         case IDC_NEWFILE:
              SendDlgItemMessage(hDlg,IDC_LISTFILE,LB_RESETCONTENT,0,0);
              tabpos[0] = 24;       /* index */
              tabpos[1] = 48;       /* name */
              tabpos[2] = 96;       /* extension */
              tabpos[3] = 120;      /* handles */
              tabpos[4] = 140;      /* openmode */
              tabpos[5] = 164;      /* windows psp */
              SendDlgItemMessage(hDlg,IDC_LISTFILE,LB_SETTABSTOPS,6,(LONG)&tabpos);

              lpword = (LPWORD)GetKernelPSP();
              j = *(lpword+0x19);
              lpfindex = (LPBYTE)MAKELONG(*(lpword+0x1a),DPMISegToDescriptor(*(lpword+0x1b)));

              y = SFTFirst(&sftentry,1);
              x = 1;
              while (y && (x < 256))
                  {
                  strncpy(str1,sftentry.name,8);
                  str1[8] = 0;
                  strncpy(str2,sftentry.ext,3);
                  str2[3] = 0;
                  pstr = str3;
                  *pstr = 0;
                  for (i=0;i<j;i++)
                      if (*(lpfindex+i) == LOBYTE(sftentry.index))
                         {
                          if (pstr != str3)
                              *pstr++ = ',';
                          wsprintf(pstr,"%u",i);
                          pstr = pstr + strlen(pstr);
                         }

                  wsprintf(str,
                           "%u\t%u\t%s\t%s\t%u\t%X\t%s",
                           x,
                           sftentry.index,
                           (LPSTR)str1,
                           (LPSTR)str2,
                           sftentry.handles,
                           sftentry.open_mode,
                           (LPSTR)str3
                           );

                  SendDlgItemMessage(hDlg,IDC_LISTFILE,LB_ADDSTRING,0,(LONG)(LPSTR)str);
                  x++;
                  y = SFTNext(&sftentry);
                  }
              break;
         }
      rc = TRUE;
      break;
    case WM_MOVE:
        if  (!IsIconic(hDlg))
           {
            RECT rect;
            GetWindowRect(hDlg,&rect);
            xFilepos = rect.left;
            yFilepos = rect.top;
           }
        break;
    case WM_CLOSE:
//      EnableMenuItem(hMenu,IDC_FILES,MF_BYCOMMAND | MF_ENABLED);
      DestroyWindow(hDlg);
      rc = TRUE;
      break;
    case WM_INITDIALOG:
      SendMessage(hDlg,WM_COMMAND,IDC_NEWFILE,0);
      SetWindowPos(hDlg,
                   0,
                   xFilepos,
                   yFilepos,
                   0,
                   0,
                   SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
      rc = TRUE;
      break;
    }
 return rc;
}
