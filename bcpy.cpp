//--------------------------------------------------------------------
//
// bcpy.cpp
//
// C++ code for BCPY utility program.  Copies the contents of a
// directory to another directory with various options.
//
//--------------------------------------------------------------------
//
// (C) Copyright 1985-2019 Ammon R. Campbell.
//
// I wrote this code for use in my own educational and experimental
// programs, but you may also freely use it in yours as long as you
// abide by the following terms and conditions:
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//   * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above
//     copyright notice, this list of conditions and the following
//     disclaimer in the documentation and/or other materials
//     provided with the distribution.
//   * The name(s) of the author(s) and contributors (if any) may not
//     be used to endorse or promote products derived from this
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
// OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.  IN OTHER WORDS, USE AT YOUR OWN RISK, NOT OURS.  
//
//--------------------------------------------------------------------

//----------------------------------------------------------
// INCLUDES
//----------------------------------------------------------

#include "util.h"
#include "filetree.h"

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <direct.h>
#include <time.h>

//----------------------------------------------------------
// MACROS
//----------------------------------------------------------

// Define the symbol DBG to enable debug/trace output to console.
#define DBG

// Maximum length of pathname string.
#ifndef MAXPATH
#define MAXPATH   512
#endif //MAXPATH

// Bit flags for dwUser field of directory entries.
#define USERFLAG_EXISTSINSOURCE  0x0001

//----------------------------------------------------------
// FORWARD PROTOTYPES
//----------------------------------------------------------

static int ParseArgument(_TCHAR *szArg);

//----------------------------------------------------------
// CLASSES
//----------------------------------------------------------

// Structure for accumulators to keep track of how many files,
// directories, and bytes were copied.
class CTotals
{
public:
   double   dBytesCopied;
   double   dBytesAlreadyExist;
   int      iFilesCopied;
   int      iFilesAlreadyExist;
   int      iDirsCopied;
   int      iDirsCreated;
   int      iDirsAlreadyExist;

   int      iSourceFilesDeleted;
   int      iSourceDirsDeleted;
   double   dSourceBytesDeleted;

   int      iDestFilesDeleted;
   int      iDestDirsDeleted;
   double   dDestBytesDeleted;

   int      iNumErrors;
   int      iNumWarnings;

public:
   CTotals()
   {
      dBytesCopied = dBytesAlreadyExist = 0.0;
      iFilesCopied = iDirsCopied = iDirsCreated = iDirsAlreadyExist = iFilesAlreadyExist = 0;
      iSourceFilesDeleted = iSourceDirsDeleted = 0;
      dSourceBytesDeleted = 0.0;
      iDestFilesDeleted = iDestDirsDeleted = 0;
      dDestBytesDeleted = 0.0;
      iNumErrors = iNumWarnings = 0;
   }
};

// Container class for the program's settings.
class CSettings
{
public:

   // Pathname of source directory.
   _TCHAR szSource[MAXPATH];

   // Pathname of destination directory.
   _TCHAR szDest[MAXPATH];

   // Pathname of log output file (if any).
   _TCHAR szLogFile[MAXPATH];

   // List of wildcard filenames to match.
   std::vector<std::wstring> cWilds;

   // Only copy files newer than this date.
   // Year will be -1 if this feature was not requested by user.
   int iNewerYear;
   int iNewerMonth;
   int iNewerDay;

   // Only copy files older than this date.
   // Year will be -1 if this feature was not requested by user.
   int iOlderYear;
   int iOlderMonth;
   int iOlderDay;

   // List of include strings.  Only files whose absolute
   // pathnames contain one or more of these substrings are
   // copied.  If this is empty, then all files are included.
   std::vector<std::wstring> cIncludes;

   // List of exclude strings.  Any files whose aboslute
   // pathnames contain one or more of these substrings
   // are not copied.  If this is empty, then no files
   // are excluded.
   std::vector<std::wstring> cExcludes;

   // If true, output debugging info.
   bool bDebug;

   // If true, verbose output is enabled.
   bool bVerbose;

   // If true, when the same file already exists in both
   // source and destination, only copy the file if the
   // source file has a different date or size than the
   // destination file.
   bool bUpdate;

   // If true, contents of files are verified after copying.
   bool bVerify;

   // If true, program will continue after an error occurs.
   bool bContinueAfterError;

   // If true, program will not display filenames as files
   // are copied.
   bool bQuiet;

   // Do everything else, but don't copy the source files
   // to the destination.
   bool bNoCopy;

   // If true, the full source and destination path of each
   // copied file is displayed.
   bool bShowPath;

   // If true, displays list of files that would be copied
   // rather than actually copying them.
   bool bList;

   // If true, enables copying of hidden and system files.
   bool bHidden;

   // If true, enables overwriting of read-only, hidden, and system
   // files in the destination.
   bool bOverwrite;

   // If true, deletes original files after copying them.
   bool bMove;

   // If true, deletes any files in the destination that
   // don't exist in the source.
   bool bClean;

   // If true, waits for a keypress before copying anything.
   bool bWait;

   // If true, specifies that the destination specified is a "root"
   // path to which the full path of the source files are appended
   // to make the destination paths.
   // Example:
   //    BCPY /ROOT C:\MYFILES\STUFF D:\
   // ...is the same as:
   //    BCPY C:\MYFILES\STUFF D:\MYFILES\STUFF
   // Example:
   //    BCPY /ROOT C:\MYFILES\STUFF D:\BACKUP\CDRIVE
   // ...would copy to D:\BACKUP\CDRIVE\MYFILES\STUFF
   bool bRoot;

   // If true, selects a low priority for the process.
   bool bPriorityLow;

public:

   // Set all member variables to desired 'default' states.
   void
   Defaults(void)
   {
      szSource[0] = '\0';
      szDest[0] = '\0';
      szLogFile[0] = '\0';
      cWilds.clear();
      iNewerYear = iNewerMonth = iNewerDay = -1;
      iOlderYear = iOlderMonth = iOlderDay = -1;
      cIncludes.clear();
      cExcludes.clear();
      bDebug = false;
      bVerbose = false;
      bUpdate = false;
      bVerify = false;
      bContinueAfterError = false;
      bQuiet = false;
      bNoCopy = false;
      bShowPath = false;
      bList = false;
      bHidden = false;
      bOverwrite = false;
      bMove = false;
      bClean = false;
      bWait = false;
      bRoot = false;
      bPriorityLow = false;
   }

   // Default constructor.
   CSettings() { Defaults(); }
};

//----------------------------------------------------------
// DATA
//----------------------------------------------------------

// Line to output to clear the current line on the console.
const _TCHAR *pszClearLine = _T("\r                                                                               \r");

// Instantiate the program's global settings.
static struct
{
   CSettings cSettings;       // Program options and settings.
   CTotals  cTotals;          // Statistics accumulators.
   CDir     cSrcTree;         // Tree of files/dirs in source.
   CDir     cDestTree;        // Tree of files/dirs in destination.
   clock_t  tStartTime;       // Time at which the program started working.
   clock_t  tLastProgress;    // Time at which the last progress update was displayed.

} Globals;

//----------------------------------------------------------
// FUNCTIONS
//----------------------------------------------------------

//
// Trim:
// Trims leading and trailing whitespace from a string.
//
static void
Trim(std::wstring &s)
{
   if (s.size() < 1)
      return;

   const _TCHAR *p = s.c_str();
   while (*p == ' ' || *p == '\t')
      p++;

   int i = static_cast<int>(_tcslen(p));
   while (i > 0 && p[i] == ' ' || p[i] == '\t')
   {
      i--;
   }

   std::wstring s2 = _T("");
   for (int j = 0; j < i; j++)
      s2 += p[i];
}

//
// logtext:
// Outputs a null-terminated character string to the
// log file, if the user specified a log file with
// the program's /LOG option.
//
// Newlines are _not_ automatically added to the end
// of each line, so the caller needs to include them.
//
static void
logtext(const _TCHAR *pszText)
{
   // Check if there is a log file.
   if (Globals.cSettings.szLogFile[0] == '\0')
      return;  // No log file defined.

   // Open the log file for writing in append mode.
   FILE *pFile = NULL;
   if (_tfopen_s(&pFile, Globals.cSettings.szLogFile, _T("a+")))
   {
      if (_tfopen_s(&pFile, Globals.cSettings.szLogFile, _T("w")))
      {
         pFile = NULL;
         _ftprintf(stderr, _T("Failed opening log file:  %s\n"), Globals.cSettings.szLogFile);
         return;
      }
   }

   // Write the text to the file.
   fwrite(pszText, _tcslen(pszText) * sizeof(_TCHAR), 1, pFile);

   fclose(pFile);
}

//
// errmsg:
// Outputs error messages to the console in a consistent format.
// If a log file is enabled, these messages are also sent to the
// log file.
//
// Parameters:
//    Name     Description
//    ----     -----------
//    pszFile  Source file in which error occurred.
//    iLine    Source line where error occurred.
//    msg      Message string to display.
//    omsg     Optional additional string (i.e. filename) to display.
//             May be NULL.
//
// Returns:
//    NONE
//
static void
errmsg(const char *pszFile, int iLine, const _TCHAR *msg, const _TCHAR *omsg = NULL)
{
   _TCHAR szText[MAXPATH * 5] = _T("");

   // Convert pszFile to wide string.
   std::wstring sFile;
   if (pszFile != NULL)
   {
      for (int i = 0; pszFile[i] != '\0'; i++)
         sFile += static_cast<_TCHAR>(pszFile[i]);
   }
   else
   {
      sFile = _T("unknown");
   }

   if (msg == NULL)
      return;
   _stprintf_s(szText, MAXPATH * 5, _T("bcpy|%s(%d):  %s"), sFile.c_str(), iLine < 1 ? 0 : iLine, msg);
   if (omsg != NULL && omsg[0] != '\0')
   {
      _tcscat_s(szText, MAXPATH * 5, _T(":  "));
      _tcscat_s(szText, MAXPATH * 5, omsg);
   }
   _tcscat_s(szText, MAXPATH * 5, _T("\n"));
   _tprintf(_T("%s"), szText);
   fflush(stdout);
   logtext(szText);
}

//
// statmsg:
// Outputs status messages to the console in a consistent format.
// If a log file is enabled, these messages are also sent to the
// log file.
//
// Parameters:
//    Name     Description
//    ----     -----------
//    msg      Message string to display.
//    omsg     Optional additional string (i.e. filename) to display.
//             May be NULL.
//
// Returns:
//    NONE
//
static void
statmsg(const _TCHAR *msg, const _TCHAR *omsg = NULL)
{
   _TCHAR szText[MAXPATH * 5];

   if (msg == NULL)
      return;
   _stprintf_s(szText, MAXPATH * 5, _T("bcpy:  %s"), msg);
   if (omsg != NULL && omsg[0] != '\0')
   {
      _tcscat_s(szText, MAXPATH * 5, _T(":  "));
      _tcscat_s(szText, MAXPATH * 5, omsg);
   }
   _tcscat_s(szText, MAXPATH * 5, _T("\n"));
   _tprintf(_T("%s"), szText);
   fflush(stdout);
   logtext(szText);
}

//
// TreeScanCallback:
// Callback function used during scanning of the source
// and destination file trees (done before any file copying
// occurs).  This gets called for each subdirectory that is
// scanned.  We use this opportunity to display the names
// of the directories as they are scanned, so the user
// doesn't think the program has crashed during a long
// scan operation.
//
// Always returns true (which tells the calling procedure
// to keep working rather than aborting).
//
static bool
TreeScanCallback(void *pContext, const _TCHAR *pszDirPath)
{
   (void)pContext;

   // Check for bogus parameters.
   if (pszDirPath == NULL || pszDirPath[0] == '\0')
      return false; // Bogus directory path.

   // If quiet mode is enabled, don't display progress.
   if (Globals.cSettings.bQuiet)
      return true;

   // Wait a bit between progress updates.
   if ((clock() - Globals.tLastProgress) < CLOCKS_PER_SEC / 4)
      return true;
   Globals.tLastProgress = clock();

   // Get directory name, and trim it if it is too long to
   // fit on one line.
   _TCHAR szOut[MAXPATH];
   int iLen = static_cast<int>(_tcslen(pszDirPath));
   if (iLen < 75)
   {
      // We will display the whole path.
      _tcscpy_s(szOut, MAXPATH, pszDirPath);
   }
   else
   {
      // We will display part of the path with "..." in front of it.
      _tcscpy_s(szOut, MAXPATH, _T("..."));
      const _TCHAR *p = _tcschr(&pszDirPath[iLen - 75], _TCHAR('\\'));
      if (p == NULL)
         p = &pszDirPath[iLen - 75];
      _tcscat_s(szOut, MAXPATH, p);
   }

   // Display name of directory currently being scanned.
   _ftprintf(stderr, pszClearLine);
   _ftprintf(stderr, _T("%s\r"), szOut);
   fflush(stderr);

   // Continue scanning.
   return true;
}

//
// CopyProgress:
// Callback function that is called during the copying or verifying
// of a file.  Updates a progress display on console.  The display
// doesn't start until 1/4 second after copying of the file begins,
// so files that copy quickly don't incur the overhead of the
// status display.
//
// pContext points to a string "C" when copying or "V" when
// verifying.  The letter in the string is used in the bar
// graph that is displayed.
//
// Always returns true (which tells the calling enumeration
// procedure to keep working rather than aborting).
//
static bool
CopyProgress(void *pContext, const _TCHAR *pszSrc, const _TCHAR *pszDest, double dBytesCopied, double dFileSize)
{
   (void)pszDest;
   (void)pszSrc;

   // If quiet mode is enabled, don't display progress.
   if (Globals.cSettings.bQuiet)
      return true;

   // Wait a bit between progress updates.
   DWORD dwTick = clock();
   if ((dwTick - Globals.tLastProgress) < CLOCKS_PER_SEC / 4)
      return true;
   Globals.tLastProgress = dwTick;

   // Context points to character to use in progress bar.
   // Typically 'C' during file copying and 'V' during
   // file verification.
   const _TCHAR *pSymbol = (const _TCHAR *)pContext;

   // Display a progress bar indicating how much of the current
   // file has been copied so far.
   _ftprintf(stderr, _T("\r"));
   for (int i = 0; i < 30; i++)
   {
      if (i < (int)(dBytesCopied * 30.0 / dFileSize))
         _ftprintf(stderr, pSymbol);
      else
         _ftprintf(stderr, _T("."));
   }

   static _TCHAR spin[] = _T("/-\\|");
   _ftprintf(stderr, _T("%c"), spin[(dwTick % CLOCKS_PER_SEC) * 4 / CLOCKS_PER_SEC]);

   _ftprintf(stderr, _T("\r"));
   fflush(stderr);

   // Keep copying.
   return true;
}

//
// QuerySource:
// Callback function to query if a given file should
// be removed from the source tree of files to be copied.
// The context pointer points to Globals.cSettings.  If the passed
// file doesn't match the program settings (includes,
// excludes, dates, etc.) then false will be returned,
// which causes the corresponding entry to be removed
// from the tree.
//
bool
QuerySource(void *pContext, const _TCHAR *pszPath, const CDirEntry *pEntry, bool bIsDir)
{
   (void)pContext;
   (void)bIsDir;

   // If includes list is non-empty, then the file must
   // match something in the includes list or be removed.
   if (Globals.cSettings.cIncludes.size() > 0)
   {
      // Check for a substring match of any of the includes with the
      // file path.
      bool bMatched = false;
      for (int i = 0; i < (int)Globals.cSettings.cIncludes.size(); i++)
      {
         if (SubstringMatch(Globals.cSettings.cIncludes[i].c_str(), pszPath))
         {
            // This file matched one of the includes.
            bMatched = true;
            break;
         }
      }
      if (!bMatched)
      {
         return false; // This file doesn't match any of the includes!
      }
   }

   // If the excludes list is non-empty, then if the file
   // matches one of the excludes, then the file needs to
   // be removed from the tree.
   if (Globals.cSettings.cExcludes.size() > 0)
   {
      // Check for a substring match of any of the excludes with the
      // file path.
      for (int i = 0; i < (int)Globals.cSettings.cExcludes.size(); i++)
      {
         if (SubstringMatch(Globals.cSettings.cExcludes[i].c_str(), pszPath))
         {
            // This file matched one of the excludes.
            return false;
         }
      }
   }

   // If wildcards were given on command line, remove anything
   // from the source tree that doesn't match one of the
   // wildcards.
   if (Globals.cSettings.cWilds.size() > 0)
   {
      // Check for a wildcard match of any of the cWilds[] with the
      // file path.
      bool bMatched = false;
      for (int i = 0; i < (int)Globals.cSettings.cWilds.size(); i++)
      {
         if (WildcardMatch(Globals.cSettings.cWilds[i].c_str(), pszPath))
         {
            // This file matched one of the wildcards.
            bMatched = true;
            break;
         }
      }
      if (!bMatched)
      {
         return false; // This file doesn't match any of the wildcards!
      }
   }

   // If new/old dates were given on the command line, remove
   // any files from the source three that are outside the
   // given date range.
   if (Globals.cSettings.iOlderYear != -1 || Globals.cSettings.iNewerYear != -1)
   {
      // Convert file time to usable form.
      SYSTEMTIME stTime;
      FileTimeToSystemTime(&pEntry->ftLastWrite, &stTime);

      // If file must be older than a certain date, check it.
      if (Globals.cSettings.iOlderYear != -1)
      {
         if (stTime.wYear > Globals.cSettings.iOlderYear)
            return false;
         else if (stTime.wYear == Globals.cSettings.iOlderYear)
         {
            if (stTime.wMonth > Globals.cSettings.iOlderMonth)
               return false;
            else if (stTime.wMonth == Globals.cSettings.iOlderMonth)
            {
               if (stTime.wDay > Globals.cSettings.iOlderDay)
                  return false;
            }
         }
      }

      // If file must be newer than a certain date, check it.
      if (Globals.cSettings.iNewerYear != -1)
      {
         if (stTime.wYear < Globals.cSettings.iNewerYear)
            return false;
         else if (stTime.wYear == Globals.cSettings.iNewerYear)
         {
            if (stTime.wMonth < Globals.cSettings.iNewerMonth)
               return false;
            else if (stTime.wMonth == Globals.cSettings.iNewerMonth)
            {
               if (stTime.wDay < Globals.cSettings.iNewerDay)
                  return false;
            }
         }
      }
   }

   // If the bHidden option isn't enabled, remove the file
   // from the tree if it has the hidden or system attributes.
   if (!Globals.cSettings.bHidden)
   {
      if ((pEntry->dwAttrib & FILE_ATTRIBUTE_HIDDEN) ||
          (pEntry->dwAttrib & FILE_ATTRIBUTE_SYSTEM))
      {
         return false;
      }
   }

   // Keep this file.
   return true;
}

//
// EnumDisplay:
// Enumeration callback function to display pathname of
// file that will be copied.
//
bool
EnumDisplay(void *pContext, const _TCHAR *pszPath, const CDirEntry *pEntry, bool bIsDir)
{
   (void)pContext;
   (void)pEntry;

   if (bIsDir)
      _tprintf(_T("  [%s]\n"), pszPath);
   else
      _tprintf(_T("  %s\n"), pszPath);
   return true;
}

//
// EnumDebugShowNodeInfo:
// Enumeration callback function to display information about
// a file or directory in one of the file trees.
//
bool
EnumDebugShowNodeInfo(void *pContext, const _TCHAR *pszPath, const CDirEntry *pEntry, bool bIsDir)
{
   (void)pContext;

   if (bIsDir)
      _tprintf(_T("d "));
   else
      _tprintf(_T("f "));
   _tprintf(_T("S:%10.0f "), pEntry->dBytes);
   _tprintf(_T("A:%08X "), pEntry->dwAttrib);
   _tprintf(_T("U:%08X "), pEntry->dwUser);
   _tprintf(_T("LW:%08X:%08X "), pEntry->ftLastWrite.dwLowDateTime, pEntry->ftLastWrite.dwHighDateTime);
   _tprintf(_T("N:%-40s "), pEntry->sName.c_str());
   _tprintf(_T("\n"));
   _tprintf(_T("    P: '%s'\n"), pszPath);
   return true;
}

//
// EnumCheckDest:
// Enumeration callback function to mark all destination
// files that exist in the source file tree.
//
bool
EnumCheckDest(void *pContext, const _TCHAR *pszPath, const CDirEntry *pEntry, bool bIsDir)
{
   (void)pContext;
   (void)pEntry;
   (void)bIsDir;

   const _TCHAR *pszRelPath = pszPath + _tcslen(Globals.cSettings.szSource) + ((Globals.cSettings.szSource[_tcslen(Globals.cSettings.szSource) - 1] == '\\') ? 0 : 1);
   CDirEntry *pDestEntry = Globals.cDestTree.FileExists(pszRelPath);
   if (pDestEntry != NULL)
      pDestEntry->dwUser |= USERFLAG_EXISTSINSOURCE;

   return true;
}

//
// EnumDelDir:
// Enumeration callback function to delete directories from
// the source tree.  This is used after the source has been
// copied to the destination if the bMove option is enabled.
// The directories are presumed to be empty at this stage.
//
bool
EnumDelDir(void *pContext, const _TCHAR *pszPath, const CDirEntry *pEntry, bool bIsDir)
{
   (void)pContext;
   (void)pEntry;

   if (!bIsDir)
      return true;   // Not a directory, so just continue.

   // Delete the directory.
   if (!Globals.cSettings.bQuiet)
      statmsg(_T("Deleting directory"), pszPath);
   if (_trmdir(pszPath))
   {
      statmsg(_T("Warning: Couldn't delete original directory"), pszPath);
      Globals.cTotals.iNumWarnings++;
   }
   Globals.cTotals.iSourceDirsDeleted++;

   // Keep processing.
   return true;
}

//
// EnumDelTagged:
// Enumeration callback function to delete files and directories
// in the destination tree that have the USERFLAG_EXISTSINSOURCE
// flag set in their dwUser flags.
//
bool
EnumDelTagged(void *pContext, const _TCHAR *pszPath, const CDirEntry *pEntry, bool bIsDir)
{
   (void)pContext;

   // Don't delete the destination root, which sometimes doesn't
   // get tagged (e.g. if it doesn't exist yet during the initial
   // scan, for example).
   if (_tcsicmp(pszPath, Globals.cSettings.szDest) == 0)
      return true;

   // Delete the file.
   if (!(pEntry->dwUser & USERFLAG_EXISTSINSOURCE))
   {
      if (!Globals.cSettings.bQuiet)
         statmsg(_T("Deleting"), pszPath);
      if (bIsDir)
      {
         if (_trmdir(pszPath))
         {
            statmsg(_T("Warning: Couldn't delete directory"), pszPath);
            Globals.cTotals.iNumWarnings++;
         }
         Globals.cTotals.iDestDirsDeleted++;
      }
      else
      {
         // Try to delete the file.
         if (_tunlink(pszPath))
         {
            // Couldn't delete the file, so try to turn off readonly, system, and hidden flags.
            DWORD dwTmp = pEntry->dwAttrib & (~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM));
            if (SetFileAttributes(pszPath, dwTmp) == INVALID_FILE_ATTRIBUTES)
            {
               statmsg(_T("Warning:  Failed changing existing read-only or hidden or system file to writable"), pszPath);
               Globals.cTotals.iNumWarnings++;
            }

            // Try to delete it again.
            if (_tunlink(pszPath))
            {
               statmsg(_T("Warning: Couldn't delete file"), pszPath);
               Globals.cTotals.iNumWarnings++;
            }
         }
         Globals.cTotals.iDestFilesDeleted++;
         Globals.cTotals.dDestBytesDeleted += pEntry->dBytes;
      }
   }

   // Keep processing.
   return true;
}

//
// EnumCopy:
// Enumeration callback function to copy one of the source files
// to the destination.
//
bool
EnumCopy(void *pContext, const _TCHAR *pszPath, const CDirEntry *pEntry, bool bIsDir)
{
   (void)pContext;

   // Build pathname of destination file or directory.
   if (_tcsnicmp(Globals.cSettings.szSource, pszPath, _tcslen(Globals.cSettings.szSource)) != 0)
   {
      errmsg(__FILE__, __LINE__, _T("Internal error; bad prefix on source path"), pszPath);
      Globals.cTotals.iNumErrors++;
      return false;
   }
   _TCHAR szNewPath[MAXPATH];
   _tcscpy_s(szNewPath, MAXPATH, Globals.cSettings.szDest);
   if (szNewPath[_tcslen(szNewPath) - 1] != '\\')
      _tcscat_s(szNewPath, MAXPATH, _T("\\"));
   const _TCHAR *pszRelPath = pszPath + _tcslen(Globals.cSettings.szSource) + ((Globals.cSettings.szSource[_tcslen(Globals.cSettings.szSource) - 1] == '\\') ? 0 : 1);
   _tcscat_s(szNewPath, MAXPATH, pszRelPath);

   // See if this file exists in the destination already.
   CDirEntry *pExists = Globals.cDestTree.FileExists(pszRelPath);
   if (pExists)
   {
      // If this name is a file in one place but a directory in
      // the other place, then complain.
      if (bIsDir && !(pExists->dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
      {
         errmsg(__FILE__, __LINE__, _T("Directory in source has same name as a file in destination"), pszRelPath);
         Globals.cTotals.iNumErrors++;
         return false;
      }
      if (!bIsDir && (pExists->dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
      {
         errmsg(__FILE__, __LINE__, _T("File in source has same name as a directory in destination"), pszRelPath);
         Globals.cTotals.iNumErrors++;
         return false;
      }
   }

   // What we do next depends on if this is a directory or a file.
   if (bIsDir)
   {
      //
      // If this directory doesn't already exist in the destination,
      // then create it.
      //
      if (!pExists)
      {
         // This directory in the destination doesn't exist yet.
         // Create the directory.
         if (!Globals.cSettings.bNoCopy)
         {
            if (!MakeDir(szNewPath))
            {
               // Failed creating it.
               errmsg(__FILE__, __LINE__, _T("Failed creating directory"), szNewPath);
               Globals.cTotals.iNumErrors++;
               if (DirExists(szNewPath))
                  errmsg(__FILE__, __LINE__, _T("...Because it already exists"), szNewPath);
               if (!Globals.cSettings.bContinueAfterError)
                  return false;
            }
            else
            {
               // The directory was created ok.
               if (Globals.cSettings.bVerbose)
                  statmsg(_T("Created directory"), szNewPath);
   
               // Copy the source dir's attributes to the destination dir.
               DWORD dwTmp = GetFileAttributes(pszPath);
               if (SetFileAttributes(szNewPath, dwTmp) == INVALID_FILE_ATTRIBUTES)
               {
                  statmsg(_T("Warning:  Failed resetting file attributes on new directory"), szNewPath);
                  Globals.cTotals.iNumWarnings++;
               }
   
               Globals.cTotals.iDirsCreated++;
            }
         }
         else // bNoCopy
         {
            // We're in no-copy mode, so if we're not in quiet
            // mode then tell the user what directory we would
            // have created if we had not been in no-copy mode.
            if (!Globals.cSettings.bQuiet)
            {
               _tprintf(_T("Would be creating directory "));
               _tprintf(_T("%s\n"), szNewPath);
            }
         }
      }
      else
      {
         // The destination directory already exists.
         Globals.cTotals.iDirsAlreadyExist++;
      }
      if (!Globals.cSettings.bNoCopy)
         Globals.cTotals.iDirsCopied++;
   }
   else
   {
      //
      // Copy this file from the source to the destination.
      //

      // If update option is enabled, and if file already exists in
      // destination, and if it has the same file timestamp and same
      // file size, then skip copying it.
      if (Globals.cSettings.bUpdate && pExists != NULL)
      {
         // If both files have the same size and same timestamp...
         // NOTE:  The 'low' portion of the timestamp is not exactly
         //        the same on NTFS and WIN32 drives, so we only
         //        compare the high bits.
         if (pEntry->dBytes == pExists->dBytes &&
            FileTimeCompare(&pEntry->ftLastWrite, &pExists->ftLastWrite) == 0
            )
         {
            // Tell the user why we're not copying this file.
            // This is not an error.
            if (Globals.cSettings.bVerbose)
               statmsg(_T("Already exists and has same size and date"), szNewPath);

            Globals.cTotals.iFilesAlreadyExist++;
            Globals.cTotals.dBytesAlreadyExist += pExists->dBytes;

            return true;
         }
      }

      // If file already exists in destination, check if it's read-only,
      // and if it is, make it writable, unless the bOverwrite option
      // isn't enabled, in which case we should display a warning about
      // not overwiting a read-only file in the destination.
#if 0
      if (pExists && (pExists->dwAttrib & FILE_ATTRIBUTE_READONLY))
#else
      if (pExists && (pExists->dwAttrib & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)))
#endif
      {
         if (Globals.cSettings.bOverwrite)
         {
            // Change file mode to make it writable, so it won't
            // fail when we try to copy over it below.
#if 0
            DWORD dwTmp = pEntry->dwAttrib & (~(FILE_ATTRIBUTE_READONLY));
#else
            DWORD dwTmp = pEntry->dwAttrib & (~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM));
#endif
            if (SetFileAttributes(szNewPath, dwTmp) == INVALID_FILE_ATTRIBUTES)
            {
               statmsg(_T("Warning:  Failed changing existing read-only or hidden or system file to writable"), szNewPath);
               Globals.cTotals.iNumWarnings++;
            }
         }
         else
         {
            // Warn user that file already exists and is read-only.
            // This is not an error.
            statmsg(_T("Warning:  Already exists and is read-only, hidden, or system"), szNewPath);
            Globals.cTotals.iNumWarnings++;
         }
      }

      // Tell the user which file we're copying, if not in quiet mode.
      if (!Globals.cSettings.bQuiet)
      {
         if (Globals.cSettings.bNoCopy)
            _tprintf(_T("Would be copying "));
         else
            _tprintf(_T("Copying "));
         if (Globals.cSettings.bShowPath)
            _tprintf(_T("%s -> %s\n"), pszPath, szNewPath);
         else
            _tprintf(_T("%s\n"), pszRelPath);
      }

      // Copy the file (unless copying is disabled).
      double dBytesCopied = 0;
      if (!Globals.cSettings.bNoCopy)
      {
         bool bCopiedOk = false;
         switch(RawCopyFileWin32(pszPath, szNewPath, &dBytesCopied, Globals.cSettings.bPriorityLow, CopyProgress, (void *)"C"))
         {
            case -1:
               errmsg(__FILE__, __LINE__, _T("Open for read failed"), pszPath);
               Globals.cTotals.iNumErrors++;
               if (!Globals.cSettings.bContinueAfterError)
                  return false;
               break;
            case -2:
               errmsg(__FILE__, __LINE__, _T("Open for write failed"), szNewPath);
               Globals.cTotals.iNumErrors++;
               if (!Globals.cSettings.bContinueAfterError)
                  return false;
               break;
            case -3:
               errmsg(__FILE__, __LINE__, _T("File write failed"), szNewPath);
               Globals.cTotals.iNumErrors++;
               if (!Globals.cSettings.bContinueAfterError)
                  return false;
               break;
            case -4:
               errmsg(__FILE__, __LINE__, _T("File read failed"), pszPath);
               Globals.cTotals.iNumErrors++;
               if (!Globals.cSettings.bContinueAfterError)
                  return false;
               break;
            case -5:
               errmsg(__FILE__, __LINE__, _T("Aborted by user"), pszPath);
               Globals.cTotals.iNumErrors++;
               return false;
            default:
               bCopiedOk = true;
         }

         if (!Globals.cSettings.bQuiet)
            _ftprintf(stderr, pszClearLine);  // To terminate line after progress report.

         // If copy of file's data succeeded above, then
         // also copy the file's timestamps and attributes.
         if (bCopiedOk)
         {
            // Retrieve the timestamps from the source file.
            FILETIME ftCreate, ftAccess, ftWrite;
            HANDLE hFile;
            hFile = CreateFile(pszPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile == NULL)
            {
               errmsg(__FILE__, __LINE__, _T("Failed opening for timestamp retrieval"), pszPath);
               Globals.cTotals.iNumErrors++;
               if (!Globals.cSettings.bContinueAfterError)
                  return false;
            }
            if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
            {
               errmsg(__FILE__, __LINE__, _T("Failed retrieving timestamp"), pszPath);
               CloseHandle(hFile);
               Globals.cTotals.iNumErrors++;
               if (!Globals.cSettings.bContinueAfterError)
                  return false;
            }
            CloseHandle(hFile);
   
            // Copy the source file's timestamps to the destination file.
            hFile = CreateFile(szNewPath, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile == NULL)
            {
               errmsg(__FILE__, __LINE__, _T("Failed opening for timestamp update"), szNewPath);
               Globals.cTotals.iNumErrors++;
               if (!Globals.cSettings.bContinueAfterError)
                  return false;
            }
            if (!SetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
            {
               errmsg(__FILE__, __LINE__, _T("Failed setting timestamp"), szNewPath);
               CloseHandle(hFile);
               Globals.cTotals.iNumErrors++;
               if (!Globals.cSettings.bContinueAfterError)
                  return false;
            }
            CloseHandle(hFile);

            // Copy the source file's attributes to the destination file.
            DWORD dwTmp = GetFileAttributes(pszPath);
            if (SetFileAttributes(szNewPath, dwTmp) == INVALID_FILE_ATTRIBUTES)
            {
               statmsg(_T("Warning:  Failed resetting file attributes"), szNewPath);
               Globals.cTotals.iNumWarnings++;
            }
   
            Globals.cTotals.iFilesCopied++;
            Globals.cTotals.dBytesCopied += dBytesCopied;
   
            // If verify option is enabled, compare the contents of the
            // source file with the destination file.
            if (Globals.cSettings.bVerify)
            {
               // Run the compare between the original and the copy.
               if (!CompareFileWin32(pszPath, szNewPath, Globals.cSettings.bPriorityLow, CopyProgress, (void *)"V"))
               {
                  // The copied file doesn't match the original!
                  errmsg(__FILE__, __LINE__, _T("Verify error; files are different"), pszRelPath);
                  Globals.cTotals.iNumErrors++;
                  if (!Globals.cSettings.bContinueAfterError)
                     return false;
               }
               if (!Globals.cSettings.bQuiet)
                  _ftprintf(stderr, pszClearLine);  // To terminate line after progress report.
            }
         }

         // If move option is enabled, then delete the original
         // source file.
         if (Globals.cSettings.bMove)
         {
            // Delete the original file.
            if (_tunlink(pszPath))
            {
               statmsg(_T("Warning: Couldn't delete original file"), pszPath);
               Globals.cTotals.iNumWarnings++;
            }
            Globals.cTotals.iSourceFilesDeleted++;
            Globals.cTotals.dSourceBytesDeleted += pEntry->dBytes;
         }

      }

   }

   // Keep copying.
   return true;
}

//
// Usage:
// Display brief summary usage information for program.
//
static void
Usage(void)
{
   printf("\
   Usage:\n\
     BCPY [options] source destination [wild...]\n\
\n\
   Where:\n\
     source       Specifies the directory to copy from.\n\
     destination  Specifies the directory to copy to.\n\
     wild         Specifies one or more optional wildcard filename\n\
                  matches.  If not specified, then all files will be\n\
                  copied.  Otherwise, only files matching the given\n\
                  wildcard(s) will be copied.\n\
\n\
   Options:\n\
     /VERIFY      Verify contents of each copied file.\n\
     /CONTINUE    Continue copying even if an error occurs.\n\
     /QUIET       Don't display filenames while copying.\n\
     /SHOWPATH    Display full source and destination filenames.\n\
     /NOCOPY      Don't copy files, but do everything else.\n\
     /UPDATE      Only copy files with different date, time, or size.\n\
     /LOG=file    Log status and error messages to specified file.\n\
");
   printf("\
     /LIST        List files that would be copied, but don't copy.\n\
     /HIDDEN      Enable copying of hidden and system files.\n\
     /OVERWRITE   Enable overwriting of read-only, hidden, and system\n\
                  files in destination.\n\
     /MOVE        Erase the original files after copying them.\n\
     /CLEAN       Erase files in destination that don't exist in source.\n\
     /WAIT        Wait for a keypress before copying.\n\
     /PRIORITYLOW Run program as a low priority process.\n\
");
   printf("\
     /ROOT        Specifies that the destination given is a \"root\" \n\
                  path to which the full path of the source files are\n\
                  appended to make the actual destination paths.\n\
                  Example: \n\
                     BCPY /ROOT C:\\MYFILES\\STUFF D:\\ \n\
                  ...is the same as: \n\
                     BCPY C:\\MYFILES\\STUFF D:\\MYFILES\\STUFF \n\
                  Example: \n\
                     BCPY /ROOT C:\\MYFILES\\STUFF D:\\BACKUP\\CDRIVE \n\
                  ...would copy to D:\\BACKUP\\CDRIVE\\MYFILES\\STUFF \n\
     /NEW=mm/dd/yyyy  or  /OLD=mm/dd/yyyy \n\
                  Only copy files newer than or older than specified date(s).\n\
     /INCLUDE={string}[,...]  or  /EXCLUDE={string}[,...]\n\
                  Include or exclude files whose absolute pathnames contain\n\
                  any of the specified substrings.\n\
     /VERBOSE     Enable verbose output.\n\
");

}

#if 0
// Not currently used

//
// ParseArgFile:
// Parses command-line arguments from a file.
//
// Parameters:
//    Name     Description
//    ----     -----------
//    fp       Open stream to read from.
//
// Returns:
//    Value    Meaning
//    -----    -------
//    1        Successful.
//    0        Error in argument or time to terminate program.
//
static int
ParseArgFile(FILE *fp)
{
   _TCHAR   szLine[512];
   _TCHAR * p;

   // Process each line of text in the file.
   while (readline(fp, szLine, 511))
   {
      // Skip leading whitespace on input line.
      p = &szLine[0];
      while (*p == ' ' || *p == '\t')
         p++;

      // Check for blank line or comment line.
      if (*p == '\0' || *p == '#' || *p == ';')
         continue;  // Line is blank or has a comment.

      // Parse the argument in the line.
      if (!ParseArgument(szLine))
      {
         return 0;
      }
   }

   // No error.
   return 1;
}

#endif

//
// ParseArgument:
// Parses a command-line argument string.
//
// Parameters:
//    Name     Description
//    ----     -----------
//    szArg    String to be parsed.
//
// Returns:
//    Value    Meaning
//    -----    -------
//    1        Successful.
//    0        Error in argument or time to terminate program.
//
static int
ParseArgument(_TCHAR *szArg)
{
   if (_tcsicmp(szArg, _T("?")) == 0 ||
       _tcsicmp(szArg, _T("-?")) == 0 ||
       _tcsicmp(szArg, _T("/?")) == 0 ||
       _tcsicmp(szArg, _T("help")) == 0)
   {
      // User wants command line help.
      Usage();
      return 0;
   }
   else if (szArg[0] == '/' || szArg[0] == '-')
   {
      // This command line argument is an option switch.
      // Figure out which one and set the corresponding option.
      if (OptionNameIs(szArg, _T("VERBOSE")) || OptionNameIs(szArg, _T("V")))
      {
         // Enable verbose output mode.
         Globals.cSettings.bVerbose = true;
      }
      else if (OptionNameIs(szArg, _T("VERIFY")))
      {
         // Enable verify mode.
         Globals.cSettings.bVerify = true;
      }
      else if (OptionNameIs(szArg, _T("DEBUG")))
      {
         // Enable debug output mode.
         Globals.cSettings.bDebug = true;
         Globals.cSettings.bVerbose = true;
      }
      else if (OptionNameIs(szArg, _T("NOCOPY")))
      {
         // Enable nocopy mode.
         Globals.cSettings.bNoCopy = true;
      }
      else if (OptionNameIs(szArg, _T("UPDATE")) || OptionNameIs(szArg, _T("U")))
      {
         // Enable update (overwrite files with different size or date) mode.
         Globals.cSettings.bUpdate = true;
      }
      else if (OptionNameIs(szArg, _T("CONTINUE")) || OptionNameIs(szArg, _T("C")))
      {
         // Enable continue-after-error mode.
         Globals.cSettings.bContinueAfterError = true;
      }
      else if (OptionNameIs(szArg, _T("QUIET")) || OptionNameIs(szArg, _T("Q")))
      {
         // Enable quiet mode.
         Globals.cSettings.bQuiet = true;
      }
      else if (OptionNameIs(szArg, _T("SHOWPATH")) || OptionNameIs(szArg, _T("S")))
      {
         // Enable show full path mode.
         Globals.cSettings.bShowPath = true;
      }
      else if (OptionNameIs(szArg, _T("LIST")) || OptionNameIs(szArg, _T("L")))
      {
         // Enable list files mode.
         Globals.cSettings.bList = true;
      }
      else if (OptionNameIs(szArg, _T("HIDDEN")) || OptionNameIs(szArg, _T("H")))
      {
         // Enable hidden/system files mode.
         Globals.cSettings.bHidden = true;
      }
      else if (OptionNameIs(szArg, _T("OVERWRITE")) || OptionNameIs(szArg, _T("O")))
      {
         // Enable overwrite-readonly mode.
         Globals.cSettings.bOverwrite = true;
      }
      else if (OptionNameIs(szArg, _T("MOVE")) || OptionNameIs(szArg, _T("M")))
      {
         // Enable move files (delete source) mode.
         Globals.cSettings.bMove = true;
      }
      else if (OptionNameIs(szArg, _T("CLEAN")))
      {
         // Enable clean destination files mode.
         Globals.cSettings.bClean = true;
      }
      else if (OptionNameIs(szArg, _T("WAIT")))
      {
         // Enable wait-for-keypress mode.
         Globals.cSettings.bWait = true;
      }
      else if (OptionNameIs(szArg, _T("LOG")))
      {
         // Set logfile name.
         _tcscpy_s(Globals.cSettings.szLogFile, MAXPATH, OptionValue(szArg));
      }
      else if (OptionNameIs(szArg, _T("ROOT")) || OptionNameIs(szArg, _T("R")))
      {
         // Enable root-path mode.
         Globals.cSettings.bRoot = true;
      }
      else if (OptionNameIs(szArg, _T("PRIORITYLOW")) || OptionNameIs(szArg, _T("P")))
      {
         // Select low priority execution.
         Globals.cSettings.bPriorityLow = true;
      }
      else if (OptionNameIs(szArg, _T("INCLUDE")))
      {
         // Add include strings.
         const _TCHAR *p = OptionValue(szArg);
         while (*p != '\0')
         {
            std::wstring s;
            s = _T("");
            while (*p == ' ' || *p == '\t' || *p == ',')
               p++;
            if (*p == '"')
            {
               while (*p != '"' && *p != '\0')
                  s += *p++;
            }
            else
            {
               while (*p != ',' && *p != '\0')
                  s += *p++;
            }
            while (*p == ' ' || *p == '\t' || *p == ',')
               p++;
            Trim(s);
            Globals.cSettings.cIncludes.push_back(s);
         }
      }
      else if (OptionNameIs(szArg, _T("EXCLUDE")))
      {
         // Add exclude strings.
         const _TCHAR *p = OptionValue(szArg);
         while (*p != '\0')
         {
            std::wstring s;
            s = _T("");
            while (*p == ' ' || *p == '\t' || *p == ',')
               p++;
            if (*p == '"')
            {
               while (*p != '"' && *p != '\0')
                  s += *p++;
            }
            else
            {
               while (*p != ',' && *p != '\0')
                  s += *p++;
            }
            while (*p == ' ' || *p == '\t' || *p == ',')
               p++;
            Trim(s);
            Globals.cSettings.cExcludes.push_back(s);
         }
      }
      else if (OptionNameIs(szArg, _T("NEW")))
      {
         // Get newer-than date setting.
         if (_tcslen(OptionValue(szArg)) >= 11 || szArg[2] != '/' || szArg[5] != '/')
         {
            errmsg(__FILE__, __LINE__, _T("Invalid date format"), szArg);
               return 0;
         }
         Globals.cSettings.iNewerMonth = _ttoi(OptionValue(szArg));
         Globals.cSettings.iNewerDay = _ttoi(&OptionValue(szArg)[3]);
         Globals.cSettings.iNewerYear = _ttoi(&OptionValue(szArg)[6]);
      }
      else if (OptionNameIs(szArg, _T("OLD")))
      {
         // Get older-than date setting.
         if (_tcslen(OptionValue(szArg)) >= 11 || szArg[2] != '/' || szArg[5] != '/')
         {
            errmsg(__FILE__, __LINE__, _T("Invalid date format"), szArg);
               return 0;
         }
         Globals.cSettings.iOlderMonth = _ttoi(OptionValue(szArg));
         Globals.cSettings.iOlderDay = _ttoi(&OptionValue(szArg)[3]);
         Globals.cSettings.iOlderYear = _ttoi(&OptionValue(szArg)[6]);
      }
      else
      {
         // Unrecognized argument!
         errmsg(__FILE__, __LINE__, _T("Unrecognized argument"), szArg);
         return 0;
      }
   }
   else
   {
      // Command line argument isn't an option switch.
      // Figure out which non-option argument it is.
      if (Globals.cSettings.szSource[0] == '\0')
      {
         // This argument is the source directory path.
         _tcscpy_s(Globals.cSettings.szSource, MAXPATH, szArg);
      }
      else if (Globals.cSettings.szDest[0] == '\0')
      {
         // This argument is the destination directory path.
         _tcscpy_s(Globals.cSettings.szDest, MAXPATH, szArg);
      }
      else
      {
         // This argument is a wildcard base filename to match.
         // Add it to the list of wildcards.
         std::wstring s = szArg;
         Globals.cSettings.cWilds.push_back(s);
      }
   }

   // No error.
   return 1;
}

//
// main:
// Application entry point.  Uses standard arguments
// and return values for a command-line program.
//
int
#ifdef _UNICODE
wmain(int argc, _TCHAR **argv)
#else
main(int argc, char **argv)
#endif
{
   // Sign on.
   const _TCHAR *pszSignon = _T("BCPY Version 3.29 (C) Copyright 1985-2008 A.R.Campbell\n");
   _tprintf(_T("%s\n"), pszSignon);

   // See if user needs help.
   if (argc < 2)
   {
      Usage();
      return EXIT_FAILURE;
   }

   // Parse command-line arguments.
   for (int n = 1; n < argc; n++)
   {
      if (!ParseArgument(argv[n]))
         return EXIT_FAILURE;
   }
   logtext(pszSignon);

   // Check for required command-line arguments.
   if (Globals.cSettings.szSource[0] == '\0')
   {
      errmsg(__FILE__, __LINE__, _T("No source directory specified"));
      return EXIT_FAILURE;
   }
   if (Globals.cSettings.szDest[0] == '\0')
   {
      errmsg(__FILE__, __LINE__, _T("No destination directory specified"));
      return EXIT_FAILURE;
   }

   // Rationalize source and destination paths.
   rationalize_path(Globals.cSettings.szSource);
   rationalize_path(Globals.cSettings.szDest);

   // If root mode specified, then add source path onto
   // destination path.
   if (Globals.cSettings.bRoot)
   {
      // Skip the drive (or \\machine) portion of the source path.
      const _TCHAR *p = &Globals.cSettings.szSource[0];
      if (p[0] == '\\' && p[1] == '\\')
      {
         p += 2;
         while (*p != '\0' && *p != '\\')
            p++;
         if (Globals.cSettings.szDest[_tcslen(Globals.cSettings.szDest) - 1] != '\\')
            _tcscat_s(Globals.cSettings.szDest, MAXPATH, _T("\\"));
         _tcscat_s(Globals.cSettings.szDest, MAXPATH, &p[1]);
      }
      else if (p[1] == ':' && p[2] == '\\')
      {
         if (Globals.cSettings.szDest[_tcslen(Globals.cSettings.szDest) - 1] != '\\')
            _tcscat_s(Globals.cSettings.szDest, MAXPATH, _T("\\"));
         _tcscat_s(Globals.cSettings.szDest, MAXPATH, &p[3]);
      }
   }

   // Display summary of options.
   if (Globals.cSettings.bVerbose)
   {
      _tprintf(_T("Options Summary:\n"));
      _tprintf(_T("  Source Directory:         %s\n"), Globals.cSettings.szSource);
      _tprintf(_T("  Destination Directory:    %s\n"), Globals.cSettings.szDest);
      if (Globals.cSettings.cWilds.size() > 0)
      {
         _tprintf(_T("  Matching:\n"));
         for (int iWild = 0; iWild < (int)Globals.cSettings.cWilds.size(); iWild++)
            _tprintf(_T("    %s\n"), Globals.cSettings.cWilds[iWild].c_str());
      }
      if (Globals.cSettings.iNewerYear != -1)
         _tprintf(_T("  Only if newer than %02d/%02d/%04d\n"), Globals.cSettings.iNewerMonth, Globals.cSettings.iNewerMonth, Globals.cSettings.iNewerYear);
      if (Globals.cSettings.iOlderYear != -1)
         _tprintf(_T("  Only if older than %02d/%02d/%04d\n"), Globals.cSettings.iOlderMonth, Globals.cSettings.iOlderMonth, Globals.cSettings.iOlderYear);
      if (Globals.cSettings.cIncludes.size() > 0)
      {
         _tprintf(_T("  Including:\n"));
         for (int i = 0; i < (int)Globals.cSettings.cIncludes.size(); i++)
            _tprintf(_T("    %s\n"), Globals.cSettings.cIncludes[i].c_str());
      }
      if (Globals.cSettings.cExcludes.size() > 0)
      {
         _tprintf(_T("  Excluding:\n"));
         for (int i = 0; i < (int)Globals.cSettings.cExcludes.size(); i++)
            _tprintf(_T("    %s\n"), Globals.cSettings.cExcludes[i].c_str());
      }
      _tprintf(_T("  Verbose output:           %s\n"), Globals.cSettings.bVerbose ? _T("yes") : _T("no"));
      _tprintf(_T("  Update if different:      %s\n"), Globals.cSettings.bUpdate ? _T("yes") : _T("no"));
      _tprintf(_T("  Verify copied files:      %s\n"), Globals.cSettings.bVerify ? _T("yes") : _T("no"));
      _tprintf(_T("  Continue after error:     %s\n"), Globals.cSettings.bContinueAfterError ? _T("yes") : _T("no"));
      _tprintf(_T("  Quiet mode:               %s\n"), Globals.cSettings.bQuiet ? _T("yes") : _T("no"));
      _tprintf(_T("  Show full paths:          %s\n"), Globals.cSettings.bShowPath ? _T("yes") : _T("no"));
      _tprintf(_T("  Show list only, no copy:  %s\n"), Globals.cSettings.bList ? _T("yes") : _T("no"));
      _tprintf(_T("  Copy hidden/system files: %s\n"), Globals.cSettings.bHidden ? _T("yes") : _T("no"));
      _tprintf(_T("  Overwrite read-only:      %s\n"), Globals.cSettings.bOverwrite ? _T("yes") : _T("no"));
      _tprintf(_T("  Move (delete after copy): %s\n"), Globals.cSettings.bMove ? _T("yes") : _T("no"));
      _tprintf(_T("  Clean destination:        %s\n"), Globals.cSettings.bClean ? _T("yes") : _T("no"));
      _tprintf(_T("  Wait before starting:     %s\n"), Globals.cSettings.bWait ? _T("yes") : _T("no"));
      _tprintf(_T("  Low priority mode:        %s\n"), Globals.cSettings.bPriorityLow ? _T("yes") : _T("no"));
   }

   // If low priority execution requested, then change priority of
   // the current process.
   if (Globals.cSettings.bPriorityLow)
   {
      SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
   }

   // Start timing.
   Globals.tStartTime = clock();
   Globals.tLastProgress = clock();

   // Scan source directory tree for all files.
   statmsg(_T("Scanning source tree"), Globals.cSettings.szSource);
   if (!Globals.cSrcTree.ScanFiles(Globals.cSettings.szSource, TreeScanCallback, NULL))
   {
      statmsg(Globals.cSrcTree.sError.c_str());
      return EXIT_FAILURE;
   }
   _ftprintf(stderr, pszClearLine);
   if (Globals.cSrcTree.cFiles.size() < 1 && Globals.cSrcTree.cDirs.size() < 1)
   {
      errmsg(__FILE__, __LINE__, _T("Nothing in source directory to copy"));
      return EXIT_FAILURE;
   }

   // Scan destination directory tree for all files.
   statmsg(_T("Scanning destination tree"), Globals.cSettings.szDest);
   if (!Globals.cDestTree.ScanFiles(Globals.cSettings.szDest, TreeScanCallback, NULL))
   {
      errmsg(__FILE__, __LINE__, Globals.cSrcTree.sError.c_str());
      return EXIT_FAILURE;
   }
   _ftprintf(stderr, pszClearLine);

   // Display scanning time.
   _tprintf(_T("Scanning Time:  %.2f Seconds\n"), (double)(clock() - Globals.tStartTime) / (double)CLOCKS_PER_SEC);

   // Mark files in dest tree that also exist in source tree.
   if (!Globals.cSrcTree.EnumFiles(Globals.cSettings.szSource, EnumCheckDest, (void *)&Globals.cSettings))
   {
      errmsg(__FILE__, __LINE__, _T("Failed enumerating files"));
      return EXIT_FAILURE;
   }

   // Remove any files from the source tree that don't match
   // the program options (e.g. excluded files, files outside
   // the specified date range, files not matching the wildcards,
   // etc.)
   if (Globals.cSettings.bVerbose)
      statmsg(_T("Pruning source tree"));
   if (!Globals.cSrcTree.PruneFiles(Globals.cSettings.szSource, QuerySource, (void *)&Globals.cSettings))
   {
      errmsg(__FILE__, __LINE__, _T("Failed pruning source file list"), Globals.cSrcTree.sError.c_str());
      return EXIT_FAILURE;
   }

   // Display summary of file counts and sizes.
   if (Globals.cSettings.bVerbose)
      statmsg(_T("Totalling"));
   {
      ENUM_COUNT_STRUCT stCounts;

      _tprintf(_T("Totals before copying:\n"));
      _tprintf(_T("  Action                Directories   Files       Bytes\n"));
      _tprintf(_T("  --------------------- ------------- ----------- ------------------\n"));

      // Count source files.
      memset(&stCounts, 0, sizeof(stCounts));
      if (!Globals.cSrcTree.EnumFiles(Globals.cSettings.szSource, EnumCallbackCountFiles, (void *)&stCounts))
      {
         errmsg(__FILE__, __LINE__, _T("Failed enumerating files"));
         return EXIT_FAILURE;
      }
      if (Globals.cSettings.szSource[_tcslen(Globals.cSettings.szSource) - 1] != '\\')
         stCounts.iNumDirs++; // Include the root.
      _TCHAR szTmp[MAXPATH];
      _TCHAR szTmp2[MAXPATH];
      _TCHAR szTmp3[MAXPATH];
      _stprintf_s(szTmp, MAXPATH, _T("%d"), stCounts.iNumDirs);
      FormatThousands(szTmp);
      _stprintf_s(szTmp2, MAXPATH, _T("%d"), stCounts.iNumFiles);
      FormatThousands(szTmp2);
      _stprintf_s(szTmp3, MAXPATH, _T("%.0f"), stCounts.dTotalBytes);
      FormatThousands(szTmp3);
      _tprintf(_T("  Source contains       %13s %11s %18s\n"),
         szTmp, szTmp2, szTmp3);

      // Count destination files.
      memset(&stCounts, 0, sizeof(stCounts));
      if (!Globals.cDestTree.EnumFiles(Globals.cSettings.szSource, EnumCallbackCountFiles, (void *)&stCounts))
      {
         errmsg(__FILE__, __LINE__, _T("Failed enumerating files"));
         return EXIT_FAILURE;
      }
      if (Globals.cSettings.szDest[_tcslen(Globals.cSettings.szDest) - 1] != '\\')
         stCounts.iNumDirs++; // Include the root.
      _stprintf_s(szTmp, MAXPATH, _T("%d"), stCounts.iNumDirs);
      FormatThousands(szTmp);
      _stprintf_s(szTmp2, MAXPATH, _T("%d"), stCounts.iNumFiles);
      FormatThousands(szTmp2);
      _stprintf_s(szTmp3, MAXPATH, _T("%.0f"), stCounts.dTotalBytes);
      FormatThousands(szTmp3);
      _tprintf(_T("  Destination contains  %13s %11s %18s\n"),
         szTmp, szTmp2, szTmp3);
   }

   // Wait for user, if enabled.
   if (Globals.cSettings.bWait)
   {
      // Prompt the user.
      _ftprintf(stderr, _T("\nBegin copying? [y/n]  "));
      fflush(stderr);
      _TCHAR c = _gettch();
      _ftprintf(stderr, _T("\n"));
      if (c != 'y' && c != 'Y')
      {
         // User said no.
         errmsg(__FILE__, __LINE__, _T("Operation aborted by user"));
         return EXIT_FAILURE;
      }
   }

   // Start timing.
   Globals.tStartTime = clock();
   Globals.tLastProgress = clock();

   // Display list of what we would copy, if list option
   // is enabled.
   if (Globals.cSettings.bList)
   {
      _tprintf(_T("Source files that would be copied:\n"));
      if (!Globals.cSrcTree.EnumFiles(Globals.cSettings.szSource, EnumDisplay, (void *)NULL))
      {
         errmsg(__FILE__, __LINE__, _T("Failed enumerating files"));
         return EXIT_FAILURE;
      }
   }

   // If debug option enabled, display debug info.
   if (Globals.cSettings.bDebug)
   {
      _tprintf(_T("------------------------------------------------------------\n"));
      _tprintf(_T("SOURCE TREE (%s)\n"), Globals.cSettings.szSource);
      _tprintf(_T("------------------------------------------------------------\n"));
      if (!Globals.cSrcTree.EnumFiles(Globals.cSettings.szSource, EnumDebugShowNodeInfo, (void *)NULL))
      {
         errmsg(__FILE__, __LINE__, _T("Failed enumerating files"));
         return EXIT_FAILURE;
      }

      _tprintf(_T("------------------------------------------------------------\n"));
      _tprintf(_T("DESTINATION TREE (%s)\n"), Globals.cSettings.szDest);
      _tprintf(_T("------------------------------------------------------------\n"));
      if (!Globals.cDestTree.EnumFiles(Globals.cSettings.szDest, EnumDebugShowNodeInfo, (void *)NULL))
      {
         errmsg(__FILE__, __LINE__, _T("Failed enumerating files"));
         return EXIT_FAILURE;
      }

      _tprintf(_T("------------------------------------------------------------\n"));
   }

   // Copy files from source to destination.
   if (!Globals.cSettings.bList)
   {
      statmsg(_T("Working"));

      // Make sure the destination directory exists.
      if (!DirExists(Globals.cSettings.szDest))
      {
         if (Globals.cSettings.bVerbose)
            statmsg(_T("Creating directory"), Globals.cSettings.szDest);
         if (!MakeDir(Globals.cSettings.szDest))
         {
            // Failed creating it.
            errmsg(__FILE__, __LINE__, _T("Failed creating directory"), Globals.cSettings.szDest);
            return EXIT_FAILURE;
         }
         Globals.cTotals.iDirsCreated++;
      }
      else
      {
         Globals.cTotals.iDirsAlreadyExist++;
      }
      Globals.cTotals.iDirsCopied++;

      //
      // Use the tree enumeration function to step through all the
      // files in the source tree.  The EnumCopy callback will do
      // all the work of copying and verifying each file.
      //
      if (!Globals.cSrcTree.EnumFiles(Globals.cSettings.szSource, EnumCopy, (void *)&Globals.cSettings))
      {
         errmsg(__FILE__, __LINE__, _T("Failed copying files"), Globals.cSrcTree.sError.c_str());
         return EXIT_FAILURE;
      }

      // If bMove option is enabled, the moved files have
      // already been deleted from the source, but the
      // moved directories need to be removed now that
      // they are empty.  Reenumerate the source tree,
      // removing the source directories as we go.
      if (Globals.cSettings.bMove)
      {
         // Delete the empty source subdiretories.
         if (!Globals.cSrcTree.EnumFiles(Globals.cSettings.szSource, EnumDelDir, (void *)&Globals.cSettings))
         {
            errmsg(__FILE__, __LINE__, _T("Failed deleting original diretories"), Globals.cSrcTree.sError.c_str());
            return EXIT_FAILURE;
         }

         // Delete the top source directory.
         if (_trmdir(Globals.cSettings.szSource))
         {
            statmsg(_T("Warning: Couldn't delete original directory"), Globals.cSettings.szSource);
            Globals.cTotals.iNumWarnings++;
         }
      }

      // Delete extra files in destination that don't exist in
      // the source tree, if bClean option enabled.
      if (Globals.cSettings.bClean)
      {
         if (!Globals.cDestTree.EnumFilesReverse(Globals.cSettings.szDest, EnumDelTagged, (void *)&Globals.cSettings))
         {
            errmsg(__FILE__, __LINE__, _T("Failed deleting files"), Globals.cDestTree.sError.c_str());
            return EXIT_FAILURE;
         }
      }
   }

   // Display totals of what was copied, what already existed
   // in the destination, etc.
   {
      _TCHAR szTmp[MAXPATH];
      _TCHAR szTmp2[MAXPATH];
      _TCHAR szTmp3[MAXPATH];

      _tprintf(_T("Completed:\n"));
      _tprintf(_T("  Action                Directories        Files       Bytes\n"));
      _tprintf(_T("  --------------------- ------------------ ----------- ------------------\n"));

      // Output totals of dirs/files/bytes copied.
      _stprintf_s(szTmp, MAXPATH, _T("%d"), Globals.cTotals.iDirsCopied);
      FormatThousands(szTmp);
      _stprintf_s(&szTmp[_tcslen(szTmp)], MAXPATH - _tcslen(szTmp), _T(" (%d new)"), Globals.cTotals.iDirsCreated);
      _stprintf_s(szTmp2, MAXPATH, _T("%d"), Globals.cTotals.iFilesCopied);
      FormatThousands(szTmp2);
      _stprintf_s(szTmp3, MAXPATH, _T("%.0f"), Globals.cTotals.dBytesCopied);
      FormatThousands(szTmp3);
      _tprintf(_T("  Copied                %18s %11s %18s\n"),
         szTmp, szTmp2, szTmp3);

      // Output totals of dirs/files/bytes not copied because they
      // already exists in the destination.
      if (Globals.cSettings.bUpdate)
      {
         _stprintf_s(szTmp, MAXPATH, _T("%d"), Globals.cTotals.iDirsAlreadyExist);
         FormatThousands(szTmp);
         _stprintf_s(szTmp2, MAXPATH, _T("%d"), Globals.cTotals.iFilesAlreadyExist);
         FormatThousands(szTmp2);
         _stprintf_s(szTmp3, MAXPATH, _T("%.0f"), Globals.cTotals.dBytesAlreadyExist);
         FormatThousands(szTmp3);
         _tprintf(_T("  Already existed       %18s %11s %18s\n"),
            szTmp, szTmp2, szTmp3);
      }

      // Output totals of dirs/files/bytes deleted from the source
      // tree after copying.
      if (Globals.cSettings.bMove)
      {
         _stprintf_s(szTmp, MAXPATH, _T("%d"), Globals.cTotals.iSourceDirsDeleted);
         FormatThousands(szTmp);
         _stprintf_s(szTmp2, MAXPATH, _T("%d"), Globals.cTotals.iSourceFilesDeleted);
         FormatThousands(szTmp2);
         _stprintf_s(szTmp3, MAXPATH, _T("%.0f"), Globals.cTotals.dSourceBytesDeleted);
         FormatThousands(szTmp3);
         _tprintf(_T("  Source deleted        %18s %11s %18s\n"),
            szTmp, szTmp2, szTmp3);
      }

      // Output totals of dirs/files/bytes deleted (cleaned) from
      // the destination tree (that were not present in the source
      // tree).
      if (Globals.cSettings.bClean)
      {
         _stprintf_s(szTmp, MAXPATH, _T("%d"), Globals.cTotals.iDestDirsDeleted);
         FormatThousands(szTmp);
         _stprintf_s(szTmp2, MAXPATH, _T("%d"), Globals.cTotals.iDestFilesDeleted);
         FormatThousands(szTmp2);
         _stprintf_s(szTmp3, MAXPATH, _T("%.0f"), Globals.cTotals.dDestBytesDeleted);
         FormatThousands(szTmp3);
         _tprintf(_T("  Destination cleaned   %18s %11s %18s\n"),
            szTmp, szTmp2, szTmp3);
      }
   }

   // Display working time.
   double dSeconds = (double)(clock() - Globals.tStartTime) / (double)CLOCKS_PER_SEC;
   _tprintf(_T("Working Time:  %.2f Seconds\n"), dSeconds);

   // Display average copying speed.
   {
      double dKBytes = Globals.cTotals.dBytesCopied / 1024.0;
      double dKBytesPerSecond = dKBytes / dSeconds;
      _TCHAR szTmp[MAXPATH];
      _stprintf_s(szTmp, MAXPATH, _T("%.2f"), dKBytesPerSecond);
      FormatThousands(szTmp);
      if (dSeconds < 1.0)
         _tprintf(_T("Average Data Rate:  Not calculated.\n"));
      else
         _tprintf(_T("Average Data Rate:  %s KBytes per second.\n"), szTmp);
   }

   // Display count of errors and warnings.
   _tprintf(_T("Completed with %d errors, %d warnings.\n"),
      Globals.cTotals.iNumErrors, Globals.cTotals.iNumWarnings);

   // Success.
   if (Globals.cSettings.bVerbose)
      statmsg(_T("Done"));
   if (Globals.cTotals.iNumErrors > 0)
      return EXIT_FAILURE;
   return EXIT_SUCCESS;
}

