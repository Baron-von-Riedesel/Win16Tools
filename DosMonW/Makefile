
# creates DosMonW.exe
# uses MSVC v1.52c

name =DosMonW
MODEL=S

MSVC=\msvc
ODIR=Build

mod1 = DOSMONW
mod2 = REGDLG
#mod3 = FILEDLG
mod4 = OPTDLG
mod5 = ABOUTDLG

CC   = $(MSVC)\bin\cl -c -A$(MODEL) -GA3s  -Oxzw -W3 -nologo -DSTRICT -Fo$* -I$(MSVC)\include -IInclude
LINK = $(MSVC)\bin\link
RC   = $(MSVC)\bin\rc
LIBS = libw toolhelp $(ODIR)\DosMon $(MODEL)libcew xlistbox userw $(MODEL)user user
LOPTS= /NOD/A:16/NON/NOE/MAP:FULL/FAR/ONERROR:NOEXE

MODULES = $(ODIR)\$(mod1).obj $(ODIR)\$(mod2).obj $(ODIR)\$(mod4).obj $(ODIR)\$(mod5).obj

$(ODIR)\$(name).exe: $(MODULES) $(ODIR)\$(name).res $(name).def
	@$(LINK) @<<
$(MODULES),
$(ODIR)\$(name).exe,
$(ODIR)\$(name).map,
$(MSVC)\lib\ Lib\ $(LIBS),
$(name).def $(LOPTS)
<<
	@$(RC) -31 -t $(ODIR)\$(name).res

$(ODIR)\$(mod1).obj: $(mod1).cpp $(name).h
	@$(CC) $(mod1).cpp

$(ODIR)\$(mod2).obj: $(mod2).cpp $(name).h
	@$(CC) $(mod2).cpp

#$(ODIR)\$(mod3).obj: $(mod3).cpp $(name).h
#	@$(CC) $(mod3).cpp

$(ODIR)\$(mod4).obj: $(mod4).cpp $(name).h
	@$(CC) $(mod4).cpp

$(ODIR)\$(mod5).obj: $(mod5).cpp $(name).h
	@$(CC) $(mod5).cpp

$(ODIR)\$(name).res: $(name).rc $(name).ico $(name).h dosmonx.h
	@$(RC) -r -fo$*.res -i$(MSVC)\include -iInclude $(name).rc

clean:
	@del $(ODIR)\*.obj
	@del $(ODIR)\*.res
	@del $(ODIR)\$(name).exe
	@del $(ODIR)\$(name).map
