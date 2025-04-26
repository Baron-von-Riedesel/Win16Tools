
name = gdistat

MSVC=c:\msvc
RC=$(MSVC)\bin\rc.exe
LINK=$(MSVC)\bin\link.exe

LIBS= libw
AOPT = -c -Fl$* -Fo$* -Sg -Cp -IInclude
LOPTS= /NOD /NOE /NON /A:16 /MAP:FULL /ONE:NOE

$(name).exe: $*.obj $(name).def
	@$(LINK) @<<
$(LOPTS) $*,
$*,
$*,
$(MSVC)\lib\+
$(LIBS),
$(name).def
<<
	@$(RC) -nologo -31 $*.exe

$(name).obj: $(name).asm Makefile
	@jwasm $(AOPT) $*.asm

