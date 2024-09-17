//--------------------------------------------------------------------
//
// filetree.h
//
// C++ source for CDir and CDirEntry classes, which are used for
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

#pragma once
#ifndef __FILETREE_H
#define __FILETREE_H

//----------------------------------------------------------
// INCLUDES
//----------------------------------------------------------

#include <vector>
#include <string>
#include <tchar.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

//----------------------------------------------------------
// MACROS
//----------------------------------------------------------

// Maximum length of pathname string.
#ifndef MAXPATH
#define MAXPATH   512
#endif //MAXPATH

//----------------------------------------------------------
// TYPES
//----------------------------------------------------------

// Context structure for the EnumCallbackCountFiles
typedef struct
{
   int      iNumFiles;
   int      iNumDirs;
   double   dTotalBytes;
} ENUM_COUNT_STRUCT;

//----------------------------------------------------------
// CLASSES
//----------------------------------------------------------

// Class to describe one directory entry.
class CDirEntry
{
public:
   std::wstring   sName;         // Name of file or directory (not full path).
   DWORD          dwUser;        // Application-defined value for each file.
   DWORD          dwAttrib;      // Attribute bits.
   double         dBytes;        // Size of file in bytes (when scanned).
   FILETIME       ftCreation;    // Timestamp for file creation.
   FILETIME       ftLastAccess;  // Timestamp for last file access.
   FILETIME       ftLastWrite;   // Timestamp for last file access.

public:
   CDirEntry();
};

// Class to describe the contents of a directory and its children.
class CDir
{
public:
   CDirEntry               cThis;   // Info about this directory.
   std::vector<CDirEntry>  cFiles;  // Files in this directory.
   std::vector<CDir>       cDirs;   // Subdirectories of this directory.

   std::wstring            sError;  // Error message string if ScanFiles
                                    // returns false.

public:
   CDir();
   bool PruneFiles(const _TCHAR *pszDirPath, bool (*pQuery)(void *pContext, const _TCHAR *pszPath, const CDirEntry *pEntry, bool bIsDir), void *pContext);
   bool EnumFiles(const _TCHAR *pszDirPath, bool (*pEnum)(void *pContext, const _TCHAR *pszPath, const CDirEntry *pEntry, bool bIsDir), void *pContext);
   bool EnumFilesReverse(const _TCHAR *pszDirPath, bool (*pEnum)(void *pContext, const _TCHAR *pszPath, const CDirEntry *pEntry, bool bIsDir), void *pContext);
   bool ScanFiles(const _TCHAR *pszDirPath, bool (*pFunc)(void *pContext, const _TCHAR *pszDirPath)=NULL, void *pContext=NULL);
   CDirEntry *FileExists(const _TCHAR *pszPath);
};

//----------------------------------------------------------
// FUNCTIONS
//----------------------------------------------------------

// Callback to pass to EnumFiles to count files, dirs, and total bytes in the tree.
// The context pointer should point to an ENUM_COUNT_STRUCT structure.
bool EnumCallbackCountFiles(void *pContext, const _TCHAR *pszPath, const CDirEntry *pEntry, bool bIsDir);

// Compares two FILETIMEs.  If bExact is false, the times will
// be compared for same date and close but not exact time.
// If bExact is true, the times must be exactly the same.
// Returns:
//    0  t1 and t2 are the same time.
//    1  t1 is after t2.
//    -1 t1 is before t2.
int FileTimeCompare(const FILETIME *t1, const FILETIME *t2, bool bExact=false);


#endif //__FILETREE_H

