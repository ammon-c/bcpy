//--------------------------------------------------------------------
//
// filetree.cpp
//
// C++ code for CDir and CDirEntry classes, which are used for
// constructing n-nodal trees of directory/filename structure in
// memory.
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

#include "filetree.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <vector>
#include <conio.h>
#include <direct.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

//----------------------------------------------------------
// MACROS
//----------------------------------------------------------

// Define the symbol DBG to enable debug/trace output to console.
//#define DBG

//----------------------------------------------------------
// IMPLEMENTATION OF CLASS CDirEntry
//----------------------------------------------------------

// Default constructor:
CDirEntry::CDirEntry()
   : sName(_T("")), dwUser(0), dwAttrib(0), dBytes(0.0)
{
   memset(&ftCreation,   0, sizeof(FILETIME));
   memset(&ftLastAccess, 0, sizeof(FILETIME));
   memset(&ftLastWrite,  0, sizeof(FILETIME));
}
      
//----------------------------------------------------------
// IMPLEMENTATION OF CLASS CDir
//----------------------------------------------------------

// Default constructor.
CDir::CDir()
{
   // Nothing needed here.
}

//
// PruneFiles:
// Given a query function by the caller, removes from this directory
// (and from all its children) any files and directories whose query
// returns false.
//
bool
CDir::PruneFiles(
   const _TCHAR *pszDirPath,
   bool (*pQuery)(void *pContext, const _TCHAR *pszPath, const CDirEntry *pEntry, bool bIsDir),
   void *pContext
   )
{
#ifdef DBG
   _tprintf(_T("DEBUG:  PruneFiles(\"%s\")\n"), pszDirPath);
   fflush(stdout);
#endif

   // Check for bogus parameters.
   if (pszDirPath == nullptr || pszDirPath[0] == '\0')
   {
      sError = _T("Bad Parameter");
      return false;
   }

   // For each file in this dir...
   int iFile = 0;
   while (iFile < (int)cFiles.size())
   {
      // Build pathname of file.
      _TCHAR szSubPath[MAXPATH];
      _tcscpy_s(szSubPath, MAXPATH, pszDirPath);
      if (szSubPath[_tcslen(szSubPath) - 1] != '\\')
         _tcscat_s(szSubPath, MAXPATH, _T("\\"));
      _tcscat_s(szSubPath, MAXPATH, cFiles[iFile].sName.c_str());

      // Query if we should keep this file.
      if (pQuery(pContext, szSubPath, &cFiles[iFile], false))
         iFile++;
      else
         cFiles.erase(cFiles.begin() + iFile);
   }

   // For each subdir in this dir...
   iFile = 0;
   while (iFile < static_cast<int>(cDirs.size()))
   {
      // Build pathname of subdir.
      _TCHAR szSubPath[MAXPATH];
      _tcscpy_s(szSubPath, MAXPATH, pszDirPath);
      if (szSubPath[_tcslen(szSubPath) - 1] != '\\')
         _tcscat_s(szSubPath, MAXPATH, _T("\\"));
      _tcscat_s(szSubPath, MAXPATH, cDirs[iFile].cThis.sName.c_str());

      // Query if we should keep this dir.
      if (pQuery(pContext, szSubPath, &cDirs[iFile].cThis, true))
         iFile++;
      else
         cDirs.erase(cDirs.begin() + iFile);
   }

   // For each subdir in this dir...
   for (int iiFile = 0; iiFile < static_cast<int>(cDirs.size()); iiFile++)
   {
      // Build pathname of subdir.
      _TCHAR szSubPath[MAXPATH];
      _tcscpy_s(szSubPath, MAXPATH, pszDirPath);
      if (szSubPath[_tcslen(szSubPath) - 1] != '\\')
         _tcscat_s(szSubPath, MAXPATH, _T("\\"));
      _tcscat_s(szSubPath, MAXPATH, cDirs[iiFile].cThis.sName.c_str());

      // Do pruning on subdir.
      if (!cDirs[iiFile].PruneFiles(szSubPath, pQuery, pContext))
      {
         // Pruning of subdir failed!
         sError = cDirs[iiFile].sError;
         return false;
      }
   }

   return true;
}

//
// EnumFiles:
// Passes all the files and directories in this directory (and all
// its children) to an enumeration function specified by the caller.
// If the enumeration function returns false, enumeration will cease.
// Returns true if enumeration finished, false if enumeration was
// interrupted by a false return of the enumeration function.
//
bool
CDir::EnumFiles(
   const _TCHAR *pszDirPath,
   bool (*pEnum)(void *pContext, const _TCHAR *pszPath, const CDirEntry *pEntry, bool bIsDir),
   void *pContext
   )
{
#ifdef DBG
   _tprintf(_T("DEBUG:  EnumFiles(\"%s\")\n"), pszDirPath);
   fflush(stdout);
#endif

   // Check for bogus parameters.
   if (pszDirPath == nullptr || pszDirPath[0] == '\0')
   {
      sError = _T("Bad Parameter");
      return false;
   }

   // For each file in this dir...
   for (int iFile = 0; iFile < static_cast<int>(cFiles.size()); iFile++)
   {
      if (cFiles[iFile].sName.size() < 1)
         continue;   // Filename is zero length, so skip it.

      // Build pathname of file.
      _TCHAR szSubPath[MAXPATH];
      _tcscpy_s(szSubPath, MAXPATH, pszDirPath);
      if (szSubPath[_tcslen(szSubPath) - 1] != '\\')
         _tcscat_s(szSubPath, MAXPATH, _T("\\"));
      _tcscat_s(szSubPath, MAXPATH, cFiles[iFile].sName.c_str());

      if (!pEnum(pContext, szSubPath, &cFiles[iFile], false))
         return false;
   }

   // For each subdir in this dir...
   for (int iFile = 0; iFile < static_cast<int>(cDirs.size()); iFile++)
   {
      // Build pathname of subdir.
      _TCHAR szSubPath[MAXPATH];
      _tcscpy_s(szSubPath, MAXPATH, pszDirPath);
      if (szSubPath[_tcslen(szSubPath) - 1] != '\\')
         _tcscat_s(szSubPath, MAXPATH, _T("\\"));
      _tcscat_s(szSubPath, MAXPATH, cDirs[iFile].cThis.sName.c_str());

      if (!pEnum(pContext, szSubPath, &cDirs[iFile].cThis, true))
         return false;

      // Enumerate children of this dir.
      if (!cDirs[iFile].EnumFiles(szSubPath, pEnum, pContext))
         return false;
   }

   return true;
}

//
// EnumFilesReverse:
// Same as EnumFiles, except goes in reverse order with files in
// this dir, then child subdirs, then subdirs.
//
// Passes all the files and directories in this directory (and all
// its children) to an enumeration function specified by the caller.
// If the enumeration function returns false, enumeration will cease.
// Returns true if enumeration finished, false if enumeration was
// interrupted by a false return of the enumeration function.
//
bool
CDir::EnumFilesReverse(
   const _TCHAR *pszDirPath,
   bool (*pEnum)(void *pContext, const _TCHAR *pszPath, const CDirEntry *pEntry, bool bIsDir),
   void *pContext
   )
{
#ifdef DBG
   _tprintf(_T("DEBUG:  EnumFilesReverse(\"%s\")\n"), pszDirPath);
   fflush(stdout);
#endif

   // Check for bogus parameters.
   if (pszDirPath == nullptr || pszDirPath[0] == '\0')
   {
      sError = _T("Bad Parameter");
      return false;
   }

   // For each file in this dir...
   for (int iFile = 0; iFile < static_cast<int>(cFiles.size()); iFile++)
   {
      // Build pathname of file.
      _TCHAR szSubPath[MAXPATH];
      _tcscpy_s(szSubPath, MAXPATH, pszDirPath);
      if (szSubPath[_tcslen(szSubPath) - 1] != '\\')
         _tcscat_s(szSubPath, MAXPATH, _T("\\"));
      _tcscat_s(szSubPath, MAXPATH, cFiles[iFile].sName.c_str());

      if (!pEnum(pContext, szSubPath, &cFiles[iFile], false))
         return false;
   }

   // For each subdir in this dir...
   for (int iFile = 0; iFile < static_cast<int>(cDirs.size()); iFile++)
   {
      // Build pathname of subdir.
      _TCHAR szSubPath[MAXPATH];
      _tcscpy_s(szSubPath, MAXPATH, pszDirPath);
      if (szSubPath[_tcslen(szSubPath) - 1] != '\\')
         _tcscat_s(szSubPath, MAXPATH, _T("\\"));
      _tcscat_s(szSubPath, MAXPATH, cDirs[iFile].cThis.sName.c_str());

      // Enumerate children of this subdir.
      if (!cDirs[iFile].EnumFilesReverse(szSubPath, pEnum, pContext))
         return false;

      // Enumerate this subdir.
      if (!pEnum(pContext, szSubPath, &cDirs[iFile].cThis, true))
         return false;
   }

   return true;
}

//
// ScanFiles:
// Fills cFiles and cDirs with information from a directory on disk.
// Optionally allows the user to specify a callback function that
// will be called with the name of each subdirectory that is scanned
// (for updating a status display, for example).
// Returns true if successful; false if error.  The sError member
// will contain an error message if return value is false.
//
bool
CDir::ScanFiles(
   const _TCHAR *pszDirPath,
   bool (*pFunc)(void *pContext, const _TCHAR *pszDirPath),
   void *pContext
   )
{
#ifdef DBG
   _tprintf(_T("DEBUG:  ScanFiles(\"%s\")\n"), pszDirPath);
   fflush(stdout);
#endif

   // Check for bogus parameters.
   if (pszDirPath == nullptr || pszDirPath[0] == '\0')
   {
      sError = _T("Bad Parameter");
      return false;
   }

   // If callback function given, pass directory name to it.
   if (pFunc != NULL)
   {
      if (!pFunc(pContext, pszDirPath))
         return false; // Callback returned false, so abort.
   }

   // Find matches.
   WIN32_FIND_DATA   stFind;
   HANDLE            hFind;
   _TCHAR           szPathPlusWild[MAXPATH];
   _tcscpy_s(szPathPlusWild, MAXPATH, pszDirPath);
   if (szPathPlusWild[_tcslen(szPathPlusWild) - 1] != '\\')
      _tcscat_s(szPathPlusWild, MAXPATH, _T("\\"));
   _tcscat_s(szPathPlusWild, MAXPATH, _T("*.*"));
   memset(&stFind, 0, sizeof(stFind));
   hFind = FindFirstFile(szPathPlusWild, &stFind);
   if (hFind == NULL)
   {
      // Nothing in this directory.
      // This is not an error.
      return true;
   }
   do
   {
#ifdef DBG
      _tprintf(_T("DEBUG:  Found \"%s\"\n"), stFind.cFileName);
      fflush(stdout);
#endif
      // Build object describing this file or dir.
      CDirEntry cFile;
      cFile.sName = stFind.cFileName;
      cFile.dwAttrib = stFind.dwFileAttributes;
      cFile.dBytes = (double)stFind.nFileSizeHigh * 65536.0 * 65536.0 + (double)stFind.nFileSizeLow;
      cFile.ftCreation = stFind.ftCreationTime;
      cFile.ftLastAccess = stFind.ftLastAccessTime;
      cFile.ftLastWrite = stFind.ftLastWriteTime;

      // Because of bug in FindFirstFile, we have to check that
      // the filename is non-empty to make sure this match actually
      // exists.
      if (cFile.sName.size() > 0)
      {
         // Is this match a directory or file?
         if (stFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
         {
            // This match is a directory.
            // If it's not the current directory or parent directory...
            if (_tcsicmp(stFind.cFileName, _T(".")) != 0 &&
                _tcsicmp(stFind.cFileName, _T("..")) != 0)
            {
               // This match is a normal directory, so add it to the
               // list of subdirectories in this directory object.
               _TCHAR szSubPath[MAXPATH];
               _tcscpy_s(szSubPath, MAXPATH, pszDirPath);
               if (szSubPath[_tcslen(szSubPath) - 1] != '\\')
                  _tcscat_s(szSubPath, MAXPATH, _T("\\"));
               _tcscat_s(szSubPath, MAXPATH, stFind.cFileName);
               CDir cDir;
               cDir.cThis = cFile;
               cDirs.push_back(cDir);
               if (!cDirs[cDirs.size() - 1].ScanFiles(szSubPath, pFunc, pContext))
               {
                  // Failed scanning files in subdirectory.
                  sError = cDirs[cDirs.size() - 1].sError;
                  return false;
               }
            }
         }
         else
         {
            // This match is a file, so add it to the list of files
            // in this directory object.
            cFiles.push_back(cFile);
         }
      }
   }
   while (FindNextFile(hFind, &stFind));
   FindClose(hFind);

   return true;
}

//
// FileExists:
// Determines if a file or subdirectory with the given
// path (relative to the root of this directory) exists
// in the tree.
//
// If found, a pointer to the corresponding directory
// entry will be returned.  Otherwise, NULL will be
// returned.
//
CDirEntry *
CDir::FileExists(const _TCHAR *pszPath)
{
   // See if path has any prepended path.
   const _TCHAR *p = _tcsrchr(pszPath, _TCHAR('\\'));
   if (p == NULL)
   {
      // No prepended directory on the specified pathname,
      // which means it should be at this level if it exists,
      // so search the entries in this directory for the
      // specified name.
      for (int iFile = 0; iFile < static_cast<int>(cFiles.size()); iFile++)
      {
         // Is this the one we want?
         if (_tcsicmp(pszPath, cFiles[iFile].sName.c_str()) == 0)  
            return &cFiles[iFile]; // Found it!
      }
      for (int iDir = 0; iDir < (int)cDirs.size(); iDir++)
      {
         // Is this the one we want?
         if (_tcsicmp(pszPath, cDirs[iDir].cThis.sName.c_str()) == 0)  
            return &cDirs[iDir].cThis; // Found it!
      }
   }
   else
   {
      // Remove one level of prepended directory path
      // and pass it on to the next subdirectory in
      // the tree (if the next level exists).
      _TCHAR szNextBase[MAXPATH];
      memset(szNextBase, 0, sizeof(szNextBase));
      p = _tcschr(pszPath, _TCHAR('\\'));
      if (p != NULL)
         memcpy(szNextBase, pszPath, sizeof(_TCHAR) * (p - pszPath));

      // Find next subdir in dirs list.
      for (int iDir = 0; iDir < (int)cDirs.size(); iDir++)
      {
         // Is this the one we want?
         if (_tcsicmp(cDirs[iDir].cThis.sName.c_str(), szNextBase) == 0)
         {
            // Scan this subdir for the specified file.
            return cDirs[iDir].FileExists(p + 1);
         }
      }
   }

   // Didn't find it.
   return NULL;
}

//----------------------------------------------------------
// FUNCTIONS
//----------------------------------------------------------

//
// EnumCallbackCountFiles:
// Enumeration callback function for counting the
// total of files and directories in the tree.
//
bool
EnumCallbackCountFiles(
   void *pContext,
   const _TCHAR *pszFullPath,
   const CDirEntry *pEntry,
   bool bIsDir
   )
{
   (void)pszFullPath;

   ENUM_COUNT_STRUCT *pInfo = (ENUM_COUNT_STRUCT *)pContext;
   if (bIsDir)
      pInfo->iNumDirs++;
   else
      pInfo->iNumFiles++;
   pInfo->dTotalBytes += pEntry->dBytes;
   return true;
}

// Compares two FILETIMEs.
// Returns:
//    0  t1 and t2 are the same time.
//    1  t1 is after t2.
//    -1 t1 is before t2.
int
FileTimeCompare(const FILETIME *t1, const FILETIME *t2, bool bExact)
{
   // Convert FILETIMEs to usable format.
   SYSTEMTIME s1, s2;
   FileTimeToSystemTime(t1, &s1);
   FileTimeToSystemTime(t2, &s2);

// For debugging only.
/*
   printf(" S1 Y%d M%d D%d H%d M%d S%d ms%d\n",
      (int)s1.wYear, (int)s1.wMonth, (int)s1.wDay, (int)s1.wHour, (int)s1.wMinute, (int)s1.wSecond, (int)s1.wMilliseconds);
   printf(" S2 Y%d M%d D%d H%d M%d S%d ms%d\n",
      (int)s2.wYear, (int)s2.wMonth, (int)s2.wDay, (int)s2.wHour, (int)s2.wMinute, (int)s2.wSecond, (int)s2.wMilliseconds);
*/

   if (s1.wYear > s2.wYear)      return 1;
   if (s1.wYear < s2.wYear)      return -1;
   if (s1.wMonth > s2.wMonth)    return 1;
   if (s1.wMonth < s2.wMonth)    return -1;
   if (s1.wDay > s2.wDay)        return 1;
   if (s1.wDay < s2.wDay)        return -1;
   if (s1.wHour > s2.wHour)      return 1;
   if (s1.wHour < s2.wHour)      return -1;

   if (bExact)
   {
      if (s1.wMinute > s2.wMinute)              return 1;
      if (s1.wMinute < s2.wMinute)              return -1;
      if (s1.wSecond > s2.wSecond)              return 1;
      if (s1.wSecond < s2.wSecond)              return -1;
      if (s1.wMilliseconds > s2.wMilliseconds)  return 1;
      if (s1.wMilliseconds < s2.wMilliseconds)  return -1;
   }
   else
   {
      // Check +/- 3 seconds.  This is approximately how far
      // off the timestamps in FAT32 can be compared to NTFS.
      int secs1 = s1.wMinute * 60 + s1.wSecond;
      int secs2 = s2.wMinute * 60 + s2.wSecond;
      if (secs1 > secs2 + 3)                    return 1;
      if (secs1 < secs2 - 3)                    return -1;
   }

   // Times are the same.
   return 0;

}

