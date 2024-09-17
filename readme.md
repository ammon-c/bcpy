## BCPY - Bulk directory copy utility

**Description:**

BCPY is a command-line utility for making copies of directories,
with many options to control exactly what's copied and how it's
copied.  I use BCPY for making local backups and for
synchronizing frequently changed files on external drives.  One
of the handy features in BCPY that's missing in most other
directory copiers is the 'clean' option, which removes any files
from the target folder that don't exist in the source folder.  

**Language:**  C++

**Platform:**  Windows

**Build:**

Run the build script from the Windows command prompt using
Microsoft's NMAKE utility.  NMAKE is typically installed with
Microsoft Visual Studio.  

**Command Line Options:**

```
   Usage:
     BCPY [options] source destination [wild...]

   Where:
     source       Specifies the directory to copy from.
     destination  Specifies the directory to copy to.
     wild         Specifies one or more optional wildcard filename
                  matches.  If not specified, then all files will be
                  copied.  Otherwise, only files matching the given
                  wildcard(s) will be copied.

   Options:
     /VERIFY      Verify contents of each copied file.
     /CONTINUE    Continue copying even if an error occurs.
     /QUIET       Don't display filenames while copying.
     /SHOWPATH    Display full source and destination filenames.
     /NOCOPY      Don't copy files, but do everything else.
     /UPDATE      Only copy files with different date, time, or size.
     /LOG=file    Log status and error messages to specified file.
     /LIST        List files that would be copied, but don't copy.
     /HIDDEN      Enable copying of hidden and system files.
     /OVERWRITE   Enable overwriting of read-only, hidden, and system
                  files in destination.
     /MOVE        Erase the original files after copying them.
     /CLEAN       Erase files in destination that don't exist in source.
     /WAIT        Wait for a keypress before copying.
     /PRIORITYLOW Run program as a low priority process.
     /ROOT        Specifies that the destination given is a "root" 
                  path to which the full path of the source files are
                  appended to make the actual destination paths.
                  Example: 
                     BCPY /ROOT C:\MYFILES\STUFF D:\ 
                  ...is the same as: 
                     BCPY C:\MYFILES\STUFF D:\MYFILES\STUFF 
                  Example: 
                     BCPY /ROOT C:\MYFILES\STUFF D:\BACKUP\CDRIVE 
                  ...would copy to D:\BACKUP\CDRIVE\MYFILES\STUFF 
     /NEW=mm/dd/yyyy  or  /OLD=mm/dd/yyyy 
                  Only copy files newer than or older than specified date(s).
     /INCLUDE={string}[,...]  or  /EXCLUDE={string}[,...]
                  Include or exclude files whose absolute pathnames contain
                  any of the specified substrings.
     /VERBOSE     Enable verbose output.
```

**Files:**

* makefile: Build script for building BCPY and REGCOPY with Microsoft Nmake.
* bcpy.cpp: C++ source for the program's main module.
* filetree.cpp: C++ source for BCPY's file and directory storage classes.
* filetree.h: C++ header for above.
* util.cpp: C++ source for miscellaneous utility functions used by BCPY.
* util.h: C++ header for above.

* (Related) regcopy.cpp:  C++ source for REGCOPY, a program to
save a backup copy of the Windows registry as a .DAT file. 
REGCOPY writes the registry data to the filename that is given
on the command line.  

**History:**

Since this program was developed for my own use on a
sporadic, occasionally on a free weekend, every once in a
while, basically random schedule, and not for public
distribution, I haven't really kept detailed track of the
development history.  Here's a summary of what I remember:  

The 1.x versions appeared starting around 1985.  These were
written in x86 assembly language on an IBM XT clone
computer with an 8088 processor.  These versions of the
program were very simple directory-to-directory copy
programs with few command-line options.  These versions ran
on MS-DOS, starting with DOS version 2.11 if I recall
correctly.  

The 2.x versions appeared starting around 1987 or 1988. 
These started out written in a combination of C and x86
assembly language, and the assembly language portions were
slowly replaced with more and more C with each updated
version until the assembly routines were eliminated
entirely after a couple of years.  If I recall correctly,
the 2.0 version was developed with the Microsoft C Compiler
3.0, though the C compiler that was used changed several
times over the years as the program was refined and as
PC-based C compilers improved.  The 2.x versions began to
have more and more command line options and evolved into
the basic format that continued into the later 3.x
versions.  The 2.x versions ran on MS-DOS, but contained
some special code to handle some special cases when running
in a DOS console window under 16-bit or 32-bit Windows.  

The 3.x versions appeared starting around 1994 or 1995.  Version
3.0 was a conversion of the last 2.x version from C to the C++
language, and all 3.x versions are written in C++.  If I recall
correctly, version 3.0 was developed with the Win32 edition of
Microsoft Visual C++ version 2-point-something on a pre-Windows
95 system (Microsoft "Chicago" Beta of Windows 95).  The 3.x
versions of the program were also designed to run as Win32
console programs rather than the DOS/Win16 console programs of
the prior 2.x versions.  

-*- end -*-
