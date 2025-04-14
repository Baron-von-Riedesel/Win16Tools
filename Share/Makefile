
# Win31 version of fakeshare
# requires jwasm & MSVC v1.52 (link, rc)

name = share

MSVC = c:\msvc

ASM  = jwasm.exe
LINK = $(MSVC)\bin\link.exe
RC   = $(MSVC)\bin\rc.exe

ODIR = Build

AOPT = -c -nologo -Sg -Fl$* -Fo$*
LOPTS= /NOLOGO/MAP/A:16/NON/NOE

all: $(ODIR) $(ODIR)\$(name).dll $(ODIR)\$(name)a.exe

$(ODIR):
	@mkdir $(ODIR)

$(ODIR)\$(name).dll: $*.obj $(name).def Makefile
	@$(LINK) @<<
$(LOPTS) $*.obj,
$*.dll,
$*.map,,
$(name).def
<<
	@$(RC) -t -31 $*.dll

$(ODIR)\$(name).obj: $(name).asm Makefile
	@$(ASM) $(AOPT) $(name).asm

$(ODIR)\$(name)a.exe: $(name)a.asm
	@$(ASM) $(AOPT) $(name)a.asm
	@$(LINK) $(LOPTS) @<<
$*.obj,
$*.exe,
$*.map,,
$(name)a.def
<<
