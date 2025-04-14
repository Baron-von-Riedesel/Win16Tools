
typedef struct tagLEPSTRUCT {
    UINT wErrCode;
    PSTR pStr;
   } LEPSTRUCT;

/**** LogErrorParam values */

LEPSTRUCT leptab[] = {
   ERR_BAD_VALUE,             "ERR_BAD_VALUE",
   ERR_BAD_FLAGS,             "ERR_BAD_FLAGS",
   ERR_BAD_INDEX,             "ERR_BAD_INDEX",
   ERR_BAD_DVALUE,            "ERR_BAD_DVALUE",
   ERR_BAD_DFLAGS,            "ERR_BAD_DFLAGS",
   ERR_BAD_DINDEX,            "ERR_BAD_DINDEX",
   ERR_BAD_PTR,               "ERR_BAD_PTR",
   ERR_BAD_FUNC_PTR,          "ERR_BAD_FUNC_PTR",
   ERR_BAD_SELECTOR,          "ERR_BAD_SELECTOR",
   ERR_BAD_STRING_PTR,        "ERR_BAD_STRING_PTR",
   ERR_BAD_HANDLE,            "ERR_BAD_HANDLE",

   ERR_BAD_HINSTANCE,         "Invalid hInstance",
   ERR_BAD_HMODULE,           "Invalid hModule",
   ERR_BAD_GLOBAL_HANDLE,     "Invalid Global Handle",
   ERR_BAD_LOCAL_HANDLE,      "Invalid Local Handle",
   ERR_BAD_ATOM,              "Invalid Atom",
   ERR_BAD_HFILE,             "Invalid hFile",

   ERR_BAD_HWND,              "Invalid hWnd",
   ERR_BAD_HMENU,             "Invalid hMenu",
   ERR_BAD_HCURSOR,           "Invalid hCursor",
   ERR_BAD_HICON,             "Invalid hIcon",
   ERR_BAD_HDWP,              "ERR_BAD_HDWP",
   ERR_BAD_CID,               "ERR_BAD_CID",
   ERR_BAD_HDRVR,             "ERR_BAD_HDRVR",

   ERR_BAD_COORDS,            "ERR_BAD_COORDS",
   ERR_BAD_GDI_OBJECT,        "Invalid hGDIObject",
   ERR_BAD_HDC,               "ERR_BAD_HDC",
   ERR_BAD_HPEN,              "ERR_BAD_HPEN",
   ERR_BAD_HFONT,             "ERR_BAD_HFONT",
   ERR_BAD_HBRUSH,            "ERR_BAD_HBRUSH",
   ERR_BAD_HBITMAP,           "ERR_BAD_HBITMAP",
   ERR_BAD_HRGN,              "ERR_BAD_HRGN",
   ERR_BAD_HPALETTE,          "ERR_BAD_HPALETTE",
   ERR_BAD_HMETAFILE,         "ERR_BAD_HMETAFILE",
          0,""};

