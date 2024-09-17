//--------------------------------------------------------------------
//
// util.h
//
// C++ header file for miscellaneous utility functions used by the
// BCPY program.
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
#ifndef __UTIL_H
#define __UTIL_H

//----------------------------------------------------------
// INCLUDES
//----------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>

//----------------------------------------------------------
// MACROS
//----------------------------------------------------------

// Maximum length of pathname string.
#ifndef MAXPATH
#define MAXPATH   512
#endif //MAXPATH

//----------------------------------------------------------
// FUNCTIONS
//----------------------------------------------------------

bool SubstringMatch(const _TCHAR *pszSub, const _TCHAR *pszText);
bool WildcardMatch(const _TCHAR *pszSub, const _TCHAR *pszText);
bool DirExists(const _TCHAR *pszPath);
bool MakeDir(const _TCHAR *pszPath);
int RawCopyFile(const _TCHAR *pszSrc, const _TCHAR *pszDest, int *piCopied=NULL);
int RawCopyFile(const _TCHAR *pszSrc, const _TCHAR *pszDest, int *piCopied,
   bool (*pFunc)(void *pContext, const _TCHAR *pszSrc, const _TCHAR *pszDest, int iBytesCopied, int iFileSize) = NULL,
   void *pContext = NULL);
int RawCopyFileWin32(const _TCHAR *pszSrc, const _TCHAR *pszDest, double *pdCopied, bool bLowPriority,
   bool (*pFunc)(void *pContext, const _TCHAR *pszSrc, const _TCHAR *pszDest, double dBytesCopied, double dFileSize) = NULL,
   void *pContext = NULL);
bool CompareFile(const _TCHAR *pszSrc, const _TCHAR *pszDest,
   bool (*pFunc)(void *pContext, const _TCHAR *pszSrc, const _TCHAR *pszDest, int iBytesCopied, int iFileSize) = NULL,
   void *pContext = NULL);
bool CompareFileWin32(const _TCHAR *pszSrc, const _TCHAR *pszDest, bool bLowPriority,
   bool (*pFunc)(void *pContext, const _TCHAR *pszSrc, const _TCHAR *pszDest, double dBytesCopied, double dFileSize) = NULL,
   void *pContext = NULL);
void rationalize_path(_TCHAR *fn);
const _TCHAR *FindBaseFilename(const _TCHAR *pszPath);
int readline(FILE *fp, _TCHAR *s, int smax);
bool OptionNameIs(const _TCHAR *szArg, const _TCHAR *szName);
const _TCHAR *OptionValue(const _TCHAR *szArg);
void FormatThousands(_TCHAR *pszNumber);


#endif //__UTIL_H

