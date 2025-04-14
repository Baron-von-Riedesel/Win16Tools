
# create Notify.exe.
# tools used: MSVC v1.52c

name = Notify
model= S

ODIR = Build

MSVC = \msvc
CC   = $(MSVC)\bin\cl
LINK = $(MSVC)\bin\link
RC   = $(MSVC)\bin\rc
HC   = $(MSVC)\bin\hc31

COPT  = -Zp -c -A$(model) -GA -Gs -G2 -Oxw -W3 -nologo -D "STRICT" -D "WINVER=0x30a" -I$(MSVC)\include -IInclude -Fo$*
LIBS = libw toolhelp commdlg ctl3d $(model)libcew xlistbox userw user
LOPTS= /NOLOGO/NOD/A:16/NON/NOE/FAR/MAP:FULL/ONERROR:NOEXE

mod1 = notify
mod2 = nfyproc
mod3 = imoption
mod4 = imabout

ALL : $(ODIR)\$(name).exe $(ODIR)\$(name).hlp

$(ODIR)\$(name).hlp: $(name).rtf $(name).hpj
	@cd $(ODIR)
	$(HC) ..\$(name).hpj
	@cd ..

$(ODIR)\$(name).exe: $(ODIR)\$(mod1).obj $(ODIR)\$(mod2).obj $(ODIR)\$(mod3).obj $(ODIR)\$(mod4).obj \
             $(ODIR)\$(name).res $(name).def Makefile
	@$(LINK) @<<
$(LOPTS) $(ODIR)\$(mod1) + $(ODIR)\$(mod2) + $(ODIR)\$(mod3) + $(ODIR)\$(mod4),
$*.exe,
$*.map,
Lib\ $(MSVC)\Lib\ $(LIBS),
$(name).def
<<
	@$(RC) -t -31 $(ODIR)\$(name).res $*.exe

$(ODIR)\$(mod1).obj: $(mod1).cpp $(name).h letab.h leptab.h
	@$(CC) $(COPT) $(mod1).cpp

$(ODIR)\$(mod2).obj: $(mod2).cpp $(name).h
	@$(CC) $(COPT) $(mod2).cpp

$(ODIR)\$(mod3).obj: $(mod3).cpp $(name).h
	@$(CC) $(COPT) $(mod3).cpp

$(ODIR)\$(mod4).obj: $(mod4).cpp $(name).h
	@$(CC) $(COPT) $(mod4).cpp

$(ODIR)\$(name).res: $(name).rc $(name)1.ico $(name)2.ico $(name).h notifyx.h
	@$(RC) -r -i$(MSVC)\include -iInclude -fo $*.res $(name).rc

clean:
	@del $(ODIR)\*.obj
	@del $(ODIR)\$(name).exe
	@del $(ODIR)\$(name).res
