
# create DosMon.dll
# using masm/jwasm
# for link, use either MS link (MSVC 1.52) or jwlink

name = DosMon

!ifdef MSLINK
MSVC = \msvc
LINK = $(MSVC)\bin\link
RC   = $(MSVC)\bin\rc
IMPLIB= $(MSVC)\bin\implib
LOPT = /NOLOGO/MAP:FULL/NOE/NON/A:16/ONE:NOE
!else
LINK = jwlink
RC=wrc
IMPLIB= wlib
!endif

ODIR = ..\Build

all: $(ODIR) $(ODIR)\$(name).dll

$(ODIR):
	@mkdir $(ODIR)

$(ODIR)\$(name).dll: $*.obj $(name).def Makefile
!ifdef MSLINK
	@$(LINK) $(LOPT) @<<
$*.obj,
$*.dll,
$*.map,
,
$(name).def
<<
	@$(RC) -t -31 $*.dll
	@$(IMPLIB) $*.lib $(name).def
!else
	@jwlink format win dll f $* n $* op q,m=$*,implib=$* @$(name).lbc
!endif

$(ODIR)\$(name).obj: $(name).asm Makefile
	@jwasm -nologo -c -Fl$* -Fo$* -Sg $(name).asm

clean:
	@del $(ODIR)\$(name).dll
	@del $(ODIR)\$(name).obj
	@del $(ODIR)\$(name).map
	@del $(ODIR)\$(name).lst
	@del $(ODIR)\$(name).lib
