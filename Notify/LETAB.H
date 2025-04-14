
typedef struct tagLESTRUCT {
    UINT wErrCode;
    PSTR pStr;
   } LESTRUCT;

/**** LogError() values */

LESTRUCT letab[] = {
   ERR_GALLOC           ,"GlobalAlloc() failed",
   ERR_GREALLOC         ,"GlobalRealloc() failed",
   ERR_GLOCK            ,"GlobalLock() failed",
   ERR_LALLOC           ,"LocalAlloc() failed",
   ERR_LREALLOC         ,"LocalRealloc() failed",
   ERR_LLOCK            ,"LocalLock() failed",
   ERR_ALLOCRES         ,"AllocResource() failed",
   ERR_LOCKRES          ,"LockResource() failed",
   ERR_LOADMODULE       ,"LoadModule() failed",

   ERR_CREATEDLG        ,"CreateDialog() failed (LoadMenu)",
   ERR_CREATEDLG2       ,"CreateDialog() failed (CreateWindow)",
   ERR_REGISTERCLASS    ,"Registerclass() failed",
   ERR_DCBUSY           ,"Device context cache full",
   ERR_CREATEWND        ,"CreateWindow() failed (class not found)",
   ERR_STRUCEXTRA       ,"program is using unallocated space",
   ERR_LOADSTR          ,"LoadString() failed",
   ERR_LOADMENU         ,"LoadMenu() failed",
   ERR_NESTEDBEGINPAINT ,"Nested BeginPaint() calls",
   ERR_BADINDEX         ,"Bad index to extra bytes in window/class",
   ERR_CREATEMENU       ,"Unable to create menu",

   ERR_CREATEDC         ,"CreateDC()/CreateIC() failed",
   ERR_CREATEMETA       ,"CreateMetafile() failed",
   ERR_DELOBJSELECTED   ,"Program deletes bitmap selected in DC",
   ERR_SELBITMAP        ,"Bitmap already selected",
   0,"" };

