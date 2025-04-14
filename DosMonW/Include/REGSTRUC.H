
typedef struct tagREGSTRUCT       /* fuer dos monitor */
  {
    WORD        ax;
    WORD        bx;
    WORD        cx;
    WORD        dx;
    WORD        si;
    WORD        di;
    WORD        bp;
    WORD        sp;
    WORD        ds;
    WORD        es;
    WORD        ss;
    WORD        cs;
    WORD        flags;
    WORD        ip;
    WORD        res1;
    WORD        res2;
    WORD        res3;
    WORD        res4;
    WORD        user1;
    WORD        user2;
    WORD        pax;
    WORD        pbx;
    WORD        pcx;
    WORD        pdx;
  } REGSTRUCT;

typedef REGSTRUCT FAR *LPREGSTRUCT;

