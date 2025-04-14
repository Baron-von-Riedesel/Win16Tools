
# requires Jwasm & MSVC v1.52 (link, rc)

name = dosalloc

MSVC   = \msvc

ASMINC = Include

ASM  = jwasm.exe
LINK = $(MSVC)\bin\link.exe
RC   = $(MSVC)\bin\rc.exe

ODIR=Build

AOPT = -c -nologo -Fl$* -Fo$* -Sg -I$(ASMINC) -D?NOCRT
LOPTS= /NOD /NOE /NON /A:16 /MAP:FULL /ONE:NOE

all: $(ODIR) $(ODIR)\$(name).exe

$(ODIR):
	@mkdir $(ODIR)

$(ODIR)\$(name).exe: $*.obj $*.res $(name).def
	@$(LINK) $(LOPTS) $*,$*,$*,,$(name).def
	@$(RC) -31 -fe$*.exe $*.res

$(ODIR)\$(name).obj: $(name).asm $(name).inc Makefile
	@$(ASM) $(AOPT) $(name).asm

$(ODIR)\$(name).res: $(name).rc $(name).inc $(name).ico Makefile
	@$(RC) -r -fo$*.res $(name).rc

