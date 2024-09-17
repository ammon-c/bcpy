#-----------------------------------------------------------------------
# Makefile to build BCOPY and REGCOPY from the C++ source code.
# Requires Microsoft Visual Studio with C++ compiler and NMAKE utility.
# To build, run "NMAKE" from the Windows command prompt.
#-----------------------------------------------------------------------

#
# Misc macros
#
CPP=cl.exe
LINK32=link.exe
OBJ= bcpy.obj filetree.obj util.obj

#
# Compiler options
#
CDEFS=-DWIN32 -D_UNICODE -DUNICODE 
CFLAGS=-c -nologo -Gs -EHsc -Od -W4 -WX -MTd -Zi -I. -Zc:wchar_t $(CDEFS)

#
# Linker options
#
LFLAGS=/NOLOGO /DEBUG gdi32.lib user32.lib kernel32.lib advapi32.lib

#
# Inference rules
#

.SUFFIXES:
.SUFFIXES:   .cpp

.cpp.obj:
   $(CPP) $(CFLAGS) $<

#
# Targets
#

all:  bcpy.exe regcopy.exe

bcpy.exe:   $(OBJ)
   $(LINK32) /OUT:$@ $(LFLAGS) $(OBJ)

regcopy.exe:      regcopy.obj
   $(LINK32) /OUT:$@ $(LFLAGS) $**

bcpy.obj:      bcpy.cpp       filetree.h util.h
filetree.obj:  filetree.cpp   filetree.h
util.obj:      util.cpp       util.h
regcopy.obj:   regcopy.cpp

# Prepare for fresh build.
# On command line use "NMAKE clean".
clean:
   if exist *.exe del *.exe
   if exist *.obj del *.obj
   if exist *.pdb del *.pdb
   if exist *.res del *.res
   if exist *.exp del *.exp
   if exist *.lib del *.lib
   if exist *.bak del *.bak
   if exist *.ilk del *.ilk
   if exist *.out del *.out

