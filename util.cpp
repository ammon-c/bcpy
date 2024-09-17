//--------------------------------------------------------------------
//
// util.cpp
//
// C++ code for miscellaneous utility functions used by the BCPY
// program.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <direct.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

//----------------------------------------------------------
// MACROS
//----------------------------------------------------------

// Define the symbol DBG to enable debug/trace output to console.
#define DBG

// Maximum length of pathname string.
#ifndef MAXPATH
#define MAXPATH   512
#endif

// Macro to determine if a given character is a separator
// between elements of a pathname.
#define is_path_separator(a)  (((a) == '\\') || ((a) == '/') || ((a) == ':'))

//----------------------------------------------------------
// FUNCTIONS
//----------------------------------------------------------

//
// SubstringMatch:
// Determines if the given substring occurs anywhere
// in the given string.  Returns true if the substring
// occurs in the string.  The comparison is not case
// sensitive.
//
bool
SubstringMatch(const _TCHAR *pszSub, const _TCHAR *pszText)
{
   // Compare at each character in the string.
   while (*pszText != '\0')
   {
      // Check if not enough characters left to make a match.
      if (_tcslen(pszText) < _tcslen(pszSub))
         return false;  // Not enough characters left.

      // If first character matches...
      if (tolower(*pszText) == tolower(*pszSub))
      {
         // Is this a match?
         if (_tcsnicmp(pszSub, pszText, _tcslen(pszSub)) == 0)
            return true;   // Yes!
      }

      // Try the next character.
      pszText++;
   }

   // Substring not found in string.
   return false;
}

//
// check_rexp:
// Checks to see if the specified string matches the specified
// limited regular expression.  The expression may contain normal
// characters which must match exactly, '?' which matches any
// single character, '*' which matches any sequence of characters,
// [a-b] which matches any character between 'a' and 'b' inclusive,
// [abc...] which matches any of the characters between the square
// brackets.  All matches are case-insensitive.
//
// Parameters:
//   Value  Meaning
//   -----  -------
//   str    String to be tested.
//   rexp   Limited regular expression to test with.
//
//   Value  Meaning
//   -----  -------
//   1      String matched specified expression.
//   0      String doesn't match specified expression.
//   -1     Error in expression.
//
static int
check_rexp(const _TCHAR *str, const _TCHAR *rexp)
{
   int   s = 0;      // Current position in str.
   int   r = 0;      // Current position in rexp.
   int   tmp, tmp2;  // Temporary variable.

   // Compare each token of rexp to str.
   while (str[s] || rexp[r])
   {
      if (rexp[r] == '[')
      {
         // `[' is beginning of a group of characters.

         // Skip `['
         r++;

         tmp = 0;
         while (rexp[r] != ']' && rexp[r])
         {
            if (rexp[r+1] == '-')
            {
               if (tolower(str[s]) >= tolower(rexp[r]) &&
                  tolower(str[s]) <= tolower(rexp[r+2]))
               {
                  tmp = 1;
               }
               r += 3;
            }
            else if (tolower(rexp[r]) == tolower(str[s]))
            {
               tmp = 1;
               r++;
            }
            else
            {
               r++;
            }
         }

         if (rexp[r] != ']')
         {
            // No ']' at end of expression.
            return -1;
         }
         r++;

         if (!tmp)
         {
            // String char didn't match group.
            return 0;
         }

         if (str[s])
            s++;
      }
      else if (rexp[r] == '?')
      {
         // `?' means we don't care what this character is.

         // Skip `['
         r++;

         if (str[s])
            s++;
      }
      else if (rexp[r] == '*')
      {
         //
         // `*' means we don't care what str is up until
         // the next matching character.
         //

         // Skip `*'
         r++;

         if (rexp[r] == '*' || rexp[r] == '?')
         {
            // Can't have multiple wildcards in a row.
            return -1;
         }

#if 0
         //
         // If no characters are left in string, then
         // error, because * must match something.
         //
         if (!str[s])
            return 0;
#endif /* 0 */

         //
         // If no characters follow the `*' in the
         // expression, then the string automatically
         // matches.
         //
         if (!rexp[r])
         {
            // Force match `*'
            return 1;
         }

         if (str[s]
#if 1
            && tolower(str[s]) != tolower(rexp[r])
#endif
            )
         {
            // `*' must skip at least one character.
            s++;
         }
#if 0
         else
         {
            // End of string, but not end of rexp.
            return 0;
         }
#endif

         //
         // Search for last match of character following
         // the `*'.
         //
         tmp2 = -1;
         tmp = s;
         while (str[tmp])
         {
            if (tolower(str[tmp]) == tolower(rexp[r]))
               tmp2 = tmp;
            tmp++;
         }

         if (tmp2 != -1)
         {
            s = tmp2;
         }

         if (tolower(str[s]) != tolower(rexp[r]))
            return 0;

         if (str[s])
            s++;
         if (rexp[r])
            r++;
      }
      else if (rexp[r] == '/' || rexp[r] == '\\')
      {
         // Slash and backslash are interchangeable.
         if (str[s] != '/' && str[s] != '\\')
         {
            // Current characters don't match.
            return 0;
         }
         if (rexp[r])
            r++;
         if (str[s])
            s++;
      }
      else
      {
         // Regular character.
         if (tolower(str[s]) != tolower(rexp[r]))
         {
            // Current characters don't match.
            return 0;
         }
         if (rexp[r])
            r++;
         if (str[s])
            s++;
      }
   }

   return 1;
}

//
// WildcardMatch:
// Determines if the given filename matches the given
// wildcard, which can contain '?' to match any single
// character or '*' to match any series of characters,
// or [x-y] to match any characters between x and y
// inclusive, or [abc...] to match any single character
// between the square brackets.
// The comparison is otherwise not case sensitive.
//
bool
WildcardMatch(const _TCHAR *pszSub, const _TCHAR *pszText)
{
   if (check_rexp(pszText, pszSub) != 1)
      return false;
   return true;
}

//
// DirExists:
// Determines if a directory with the given path exists and is readable.
// Returns true if so, false if not or if the specified path refers to
// a file rather than a directory.
//
bool
DirExists(const _TCHAR *pszPath)
{
   DWORD dwTmp = GetFileAttributes(pszPath);
   if (dwTmp == INVALID_FILE_ATTRIBUTES)
      return false;
   if (dwTmp & FILE_ATTRIBUTE_DIRECTORY)
      return true;
   return false;
}

//
// MakeDir:
// Attempts to create the specified directory.
// May be multiple levels deep.
// Returns true if successful (or directory already exists).
// Returns false if directory couldn't be created.
//
bool
MakeDir(const _TCHAR *pszPath)
{
   _TCHAR szDir[MAX_PATH];
   int iPos = 0;

   // Process each segment of the path.
   const _TCHAR *p = pszPath;
   while (*p != '\0')
   {
      // Eat the backslash, if any.
      if (*p == '\\')
      {
         szDir[iPos++] = *p++;
         szDir[iPos] = '\0';
      }

      // Extract path segment.
      while (*p != '\0' && *p != '\\')
      {
         szDir[iPos++] = *p++;
         szDir[iPos] = '\0';
      }

      if (!DirExists(szDir))
      {
         // Make it.
         if (_tmkdir(szDir))
         {
            // Failed!
            return false;
         }
      }
   }

   return true;
}

//
// RawCopyFileWin32:
// Creates a copy of a file on disk.  This function copies
// only the contents of the file, not the timestamps or
// attributes.
//
// Optionally accepts a status callback function and context
// pointer, in which case the callback function is called
// periodically so the caller can update a status display
// during the compare operation.  If the callback function
// returns false, the operation is aborted.
//
// Returns:
//    0 = successful.
//   -1 = Failed opening file for read.
//   -2 = Failed opening file for write.
//   -3 = Failed writing file.
//   -4 = Failed reading file.
//   -5 = Status function returned false.
//
int
RawCopyFileWin32(
   const _TCHAR *pszSrc,      // File to copy from.
   const _TCHAR *pszDest,     // File to copy to.
   double *pdCopied,          // Pointer to variable to receive count of bytes copied.
   bool bLowPriority,         // True if code should allow other processes to run between file chunks read.
   bool (*pFunc)(void *pContext, const _TCHAR *pszSrc, const _TCHAR *pszDest, double dBytesCopied, double dFileSize), // Pointer to status callback function.  May be NULL.
   void *pContext             // Pointer to context pointer for status callback function.  May be NULL.
   )
{
   double dTotalBytes = 0;    // Keep track of how many bytes copied.

   // Open the input file.
   HANDLE pIn = NULL;
   pIn = CreateFile(pszSrc,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
   if (pIn == INVALID_HANDLE_VALUE)
   {
      // Failed opening file for reading.
      pIn = NULL;
      return -1;
   }

   // Determine size of input file.
   DWORD dwLow, dwHigh;
   dwLow = GetFileSize(pIn, &dwHigh);
   if (dwLow == 0xFFFFFFFF)
   {
      // Failed getting file size!
      CloseHandle(pIn);
      return -1;
   }
   double dFileLength = dwLow + (((double)dwHigh) * 65536. * 65536.);

   // Open the output file.
   HANDLE pOut = NULL;
   pOut = CreateFile(pszDest,
            GENERIC_WRITE,
            0, // No sharing.
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
   if (pOut == INVALID_HANDLE_VALUE)
   {
      // Failed opening file for write.
      pOut = NULL;
      CloseHandle(pIn);
      return -2;
   }

   // Temp storage for file copying.
   static char pBuffer[65536];

   // Start status display, if status function given.
   if (pFunc != NULL)
   {
      if (!pFunc(pContext, pszSrc, pszDest, dTotalBytes, dFileLength))
      {
         CloseHandle(pIn);
         CloseHandle(pOut);
         return -5; // Progress function wants to abort.
      }
   }

   // Read chunks until we've done the whole file.
   DWORD dwBytes = 0;
   while (ReadFile(pIn, pBuffer, 65536, &dwBytes, NULL) != 0 && (dwBytes > 0))
   {
      // Write the chunk we just read out to the output file.
      DWORD dwBytes2 = 0;
      if (WriteFile(pOut, pBuffer, dwBytes, &dwBytes2, NULL) == 0 || (dwBytes2 != dwBytes))
      {
         // Failed writing to output file!
         CloseHandle(pIn);
         CloseHandle(pOut);
         _tunlink(pszDest);
         return -3;
      }

      // Update total count of bytes copied.
      dTotalBytes += dwBytes;

      // Update status display.
      // Note that this is called very frequently, so the caller
      // may not want to update a display on _every_ callback.
      if (pFunc != NULL)
      {
         if (!pFunc(pContext, pszSrc, pszDest, dTotalBytes, dFileLength))
         {
            CloseHandle(pIn);
            CloseHandle(pOut);
            _tunlink(pszDest);
            return -5; // Progress function wants to abort.
         }
      }

      // Let other threads run.
      if (bLowPriority)
         Sleep(0);
   }

   // Finish status display, if status function given.
   if (pFunc != NULL)
   {
      if (!pFunc(pContext, pszSrc, pszDest, dTotalBytes, dFileLength))
      {
         CloseHandle(pIn);
         CloseHandle(pOut);
         _tunlink(pszDest);
         return -5; // Progress function wants to abort.
      }
   }

   // Close files.
   CloseHandle(pIn);
   CloseHandle(pOut);

   // Make sure the whole file was read.
   if (dTotalBytes != dFileLength)
   {
      // Read error!
      _tunlink(pszDest);
      return -4;
   }

   // If caller wants count of bytes copied.
   if (pdCopied != NULL)
      *pdCopied = dTotalBytes;

   // No error.
   return 0;
}

//
// RawCopyFile:
// Creates a copy of a file on disk.  This function copies
// only the contents of the file, not the timestamps or
// attributes.
//
// Optionally accepts a status callback function and context
// pointer, in which case the callback function is called
// periodically so the caller can update a status display
// during the compare operation.  If the callback function
// returns false, the operation is aborted.
//
// Returns:
//    0 = successful.
//   -1 = Failed opening file for read.
//   -2 = Failed opening file for write.
//   -3 = Failed writing file.
//   -4 = Failed reading file.
//   -5 = Status function returned false.
//
int
RawCopyFile(
   const _TCHAR *pszSrc,      // File to copy from.
   const _TCHAR *pszDest,     // File to copy to.
   int *piCopied,             // Pointer to int to receive count of bytes copied.
   bool (*pFunc)(void *pContext, const _TCHAR *pszSrc, const _TCHAR *pszDest, int iBytesCopied, int iFileSize), // Pointer to status callback function.  May be NULL.
   void *pContext             // Pointer to context pointer for status callback function.  May be NULL.
   )
{
   int iTotalBytes = 0;    // Keep track of how many bytes copied.

   // Open the input file.
   FILE *pIn = NULL;
   if (_tfopen_s(&pIn, pszSrc, _T("rb")))
   {
      // Failed opening file for reading.
      pIn = NULL;
      return -1;
   }

   // Determine size of input file.
   fseek(pIn, 0, SEEK_END);
   int iFileLength = ftell(pIn);
   fseek(pIn, 0, SEEK_SET);

   // Open the output file.
   FILE *pOut = NULL;
   if (_tfopen_s(&pOut, pszDest, _T("wb")))
   {
      // Failed opening file for write.
      pOut = NULL;
      fclose(pIn);
      return -2;
   }

   // Temp storage for file copying.
   static char pBuffer[65536];

   // Start status display, if status function given.
   if (pFunc != NULL)
   {
      if (!pFunc(pContext, pszSrc, pszDest, iTotalBytes, iFileLength))
         return -5; // Progress function wants to abort.
   }

   // Read chunks until we've done the whole file.
   int iBytes = 0;
   while ((iBytes = static_cast<int>(fread(pBuffer, 1, 65536, pIn))) > 0)
   {
      // Write the chunk we just read out to the output file.
      if (fwrite(pBuffer, 1, iBytes, pOut) != static_cast<size_t>(iBytes))
      {
         // Failed writing to output file!
         fclose(pIn);
         fclose(pOut);
         _tunlink(pszDest);
         return -3;
      }

      // Update total count of bytes copied.
      iTotalBytes += iBytes;

      // Update status display.
      // Note that this is called very frequently, so the caller
      // may not want to update a display on _every_ callback.
      if (pFunc != NULL)
      {
         if (!pFunc(pContext, pszSrc, pszDest, iTotalBytes, iFileLength))
            return -5; // Progress function wants to abort.
      }
   }

   // Finish status display, if status function given.
   if (pFunc != NULL)
   {
      if (!pFunc(pContext, pszSrc, pszDest, iTotalBytes, iFileLength))
         return -5; // Progress function wants to abort.
   }

   // Close files.
   fclose(pIn);
   fclose(pOut);

   // Make sure the whole file was read.
   if (iTotalBytes != iFileLength)
   {
      // Read error!
      _tunlink(pszDest);
      return -4;
   }

   // If caller wants count of bytes copied.
   if (piCopied != NULL)
      *piCopied = iTotalBytes;

   // No error.
   return 0;
}

//
// RawCopyFile:
// --- Same as above except no status reporting ---
// Creates a copy of a file on disk.  This function copies
// only the contents of the file, not the timestamps or
// attributes.
//
// Returns:
//    0 = successful.
//   -1 = Failed opening file for read.
//   -2 = Failed opening file for write.
//   -3 = Failed writing file.
//   -4 = Failed reading file.
//
int
RawCopyFile(
   const _TCHAR *pszSrc,      // File to copy from.
   const _TCHAR *pszDest,     // File to copy to.
   int *piCopied              // Pointer to int to receive count of bytes copied.
   )
{
   return RawCopyFile(pszSrc, pszDest, piCopied, NULL, NULL);
}

//
// CompareFileWin32:
// Compares the contents of two files.  If the contents of
// the files differ, or if either file can't be opened, or
// if one file's size differs from the other, then this
// function will return false.  Otherwise returns true.
//
// Optionally accepts a callback function and context
// pointer, in which case the callback function is called
// periodically so the caller can update a status display
// during the compare operation.  If the callback function
// returns false, the operation is aborted.
//
bool
CompareFileWin32(const _TCHAR *pszSrc, const _TCHAR *pszDest,
   bool bLowPriority,      // True if code should allow other processes to run between file chunks read.
   bool (*pFunc)(void *pContext, const _TCHAR *pszSrc, const _TCHAR *pszDest, double dBytesCopied, double dFileSize),
   void *pContext)
{
   double dTotalBytes = 0;    // Keep track of how many bytes compared.

   // Open the input file.
   HANDLE pf1 = NULL;
   pf1 = CreateFile(pszSrc,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
   if (pf1 == INVALID_HANDLE_VALUE)
   {
      // Failed opening file for reading.
      pf1 = NULL;
      return false;
   }

   // Determine size of input file.
   DWORD dwLow, dwHigh;
   dwLow = GetFileSize(pf1, &dwHigh);
   if (dwLow == 0xFFFFFFFF)
   {
      // Failed getting file size!
      CloseHandle(pf1);
      return false;
   }
   double dFileLength = dwLow + (((double)dwHigh) * 65536. * 65536.);

   // Open the second file.
   HANDLE pf2 = NULL;
   pf2 = CreateFile(pszDest,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
   if (pf2 == INVALID_HANDLE_VALUE)
   {
      // Failed opening file for write.
      pf2 = NULL;
      CloseHandle(pf1);
      return false;
   }

   // Start status display, if status function given.
   if (pFunc != NULL)
   {
      if (!pFunc(pContext, pszSrc, pszDest, dTotalBytes, dFileLength))
      {
         CloseHandle(pf1);
         CloseHandle(pf2);
         return false; // Progress function wants to abort.
      }
   }

   // Temp storage for file comparing.
   static char pBuffer1[65536];
   static char pBuffer2[65536];

   // Read chunks until we've done the whole file.
   DWORD dwBytes = 0;
   while (ReadFile(pf1, pBuffer1, 65536, &dwBytes, NULL) != 0 && (dwBytes > 0))
   {
      // Read same chunk from 2nd file.
      DWORD dwBytes2;
      if (ReadFile(pf2, pBuffer2, 65536, &dwBytes2, NULL) == 0 || (dwBytes2 != dwBytes))
      {
         // Files differ in size or read error!
         CloseHandle(pf1);
         CloseHandle(pf2);
         return false;
      }

      // Compare the two chunks.
      if (memcmp(pBuffer1, pBuffer2, dwBytes) != 0)
      {
         // Contents of files differ!
         CloseHandle(pf1);
         CloseHandle(pf2);
         return false;
      }

      // Update total count of bytes compared.
      dTotalBytes += dwBytes;

      // Update status display.
      // Note that this is called very frequently, so the caller
      // may not want to update a display on _every_ callback.
      if (pFunc != NULL)
      {
         if (!pFunc(pContext, pszSrc, pszDest, dTotalBytes, dFileLength))
         {
            CloseHandle(pf1);
            CloseHandle(pf2);
            return false; // Progress function wants to abort.
         }
      }

      // Let other threads run.
      if (bLowPriority)
         Sleep(0);
   }

   // Finish status display, if status function given.
   if (pFunc != NULL)
   {
      if (!pFunc(pContext, pszSrc, pszDest, dTotalBytes, dFileLength))
      {
         CloseHandle(pf1);
         CloseHandle(pf2);
         return false; // Progress function wants to abort.
      }
   }

   CloseHandle(pf1);
   CloseHandle(pf2);

   // Make sure the whole file was read.
   if (dTotalBytes != dFileLength)
   {
      // Read error!
      return false;
   }

   // No error.
   return true;
}

//
// CompareFile:
// Compares the contents of two files.  If the contents of
// the files differ, or if either file can't be opened, or
// if one file's size differs from the other, then this
// function will return false.  Otherwise returns true.
//
// Optionally accepts a callback function and context
// pointer, in which case the callback function is called
// periodically so the caller can update a status display
// during the compare operation.  If the callback function
// returns false, the operation is aborted.
//
bool
CompareFile(const _TCHAR *pszSrc, const _TCHAR *pszDest,
   bool (*pFunc)(void *pContext, const _TCHAR *pszSrc, const _TCHAR *pszDest, int iBytesCopied, int iFileSize),
   void *pContext)
{
   int iTotalBytes = 0;    // Keep track of how many bytes compared.

   // Open file 1.
   FILE *pf1 = NULL;
   if (_tfopen_s(&pf1, pszSrc, _T("rb")))
   {
      // Failed opening first file.
      return false;
   }

   // Determine size of file 1.
   fseek(pf1, 0, SEEK_END);
   int iFileLength = ftell(pf1);
   fseek(pf1, 0, SEEK_SET);

   // Open file 2.
   FILE *pf2 = NULL;
   if (_tfopen_s(&pf2, pszDest, _T("rb")))
   {
      // Failed opening second file.
      fclose(pf1);
      return false;
   }

   // Start status display, if status function given.
   if (pFunc != NULL)
   {
      if (!pFunc(pContext, pszSrc, pszDest, iTotalBytes, iFileLength))
         return false; // Progress function wants to abort.
   }

   // Temp storage for file comparing.
   static char pBuffer1[65536];
   static char pBuffer2[65536];

   // Read chunks until we've done the whole file.
   int iBytes = 0;
   while ((iBytes = static_cast<int>(fread(pBuffer1, 1, 65536, pf1))) > 0)
   {
      // Read same chunk from 2nd file.
      if (fread(pBuffer2, 1, iBytes, pf2) != static_cast<size_t>(iBytes))
      {
         // Files differ in size!
         fclose(pf1);
         fclose(pf2);
         return false;
      }

      // Compare the two chunks.
      if (memcmp(pBuffer1, pBuffer2, iBytes) != 0)
      {
         // Contents of files differ!
         fclose(pf1);
         fclose(pf2);
         return false;
      }

      // Update total count of bytes compared.
      iTotalBytes += iBytes;

      // Update status display.
      // Note that this is called very frequently, so the caller
      // may not want to update a display on _every_ callback.
      if (pFunc != NULL)
      {
         if (!pFunc(pContext, pszSrc, pszDest, iTotalBytes, iFileLength))
            return false; // Progress function wants to abort.
      }
   }

   // Finish status display, if status function given.
   if (pFunc != NULL)
   {
      if (!pFunc(pContext, pszSrc, pszDest, iTotalBytes, iFileLength))
         return false; // Progress function wants to abort.
   }

   // Close files.
   fclose(pf1);
   fclose(pf2);

   // Make sure the whole file was read.
   if (iTotalBytes != iFileLength)
   {
      // Read error!
      return false;
   }

   // No error.
   return true;
}

//
// rationalize_path:
// Converts a relative path to an absolute path.
//
// Parameters:
//   Name   Description
//   ----   -----------
//   fn     Path to be converted.
//
// Returns:
//   NONE
//
void
rationalize_path(_TCHAR *fn)
{
   _TCHAR *    origfn;                 // Saved pointer to 'fn'.
   _TCHAR      result[MAXPATH + 1];    // Buffer to build result in.
   int         rpos = 0;               // Position in 'result'.
   _TCHAR      cwd[MAXPATH + 1];       // Current working directory.
   _TCHAR      element[MAXPATH + 1];   // Element from path.
   int         epos;                   // Position in 'element'.

   // Check for bogus argument.
   if (fn == NULL)
      return;
   if (fn[0] == '\0')
      return;

   // Save where 'fn' points to so we can use it again later.
   origfn = fn;

   //
   // Begin by determining which drive to use.
   //
   rpos = 0;
   cwd[0] = '\0';
   if (_tcslen(fn) > 1 && fn[1] == ':')
   {
      // Use drive "d:" specified by caller.
      result[rpos++] = *fn++;
      result[rpos++] = *fn++;
      result[rpos] = '\0';

      // Get current directory for specified drive.
      _tgetdcwd(tolower(result[0]) - 'a' + 1, cwd, MAXPATH);
      if (cwd[_tcslen(cwd) - 1] != '\\')
         _tcscat_s(cwd, MAXPATH, _T("\\"));
   }
   else if (_tcslen(fn) > 1 && fn[0] == '\\' && fn[1] == '\\')
   {
      // Use drive "\\machine\drive" specified by caller.
      result[rpos++] = *fn++;
      result[rpos++] = *fn++;
      while (!is_path_separator(*fn) && *fn != '\0')
      {
         /* Copy byte of machine name. */
         result[rpos++] = *fn++;
      }
      if (*fn != '\0')
         result[rpos++] = *fn++;
      while (!is_path_separator(*fn) && *fn)
      {
         /* Copy byte of drive name. */
         result[rpos++] = *fn++;
      }
      result[rpos] = '\0';
   }
   else
   {
      // No drive specified; use current working drive.
      _tgetcwd(cwd, MAXPATH);
      if (cwd[_tcslen(cwd) - 1] != '\\')
         _tcscat_s(cwd, MAXPATH, _T("\\"));
      result[rpos++] = cwd[0];
      result[rpos++] = cwd[1];
      result[rpos] = '\0';
   }

   //
   // Determine if we should root with caller's path
   // or current working directory.
   //
   if (!is_path_separator(*fn))
   {
      //
      // Caller's path doesn't start at root, so
      // copy working directory in first.
      //
      if (_tcslen(cwd) > 2)
         _tcscat_s(result, MAXPATH, &cwd[2]);
   }

   //
   // Use relative path from specified drive.
   //
   _tcscat_s(result, MAXPATH, fn);

   //
   // Okay, now we have something in 'result' that
   // begins with a drive letter (or network drivespec),
   // and begins at the root of that drive.  However,
   // this path may still have "." and ".." elements
   // in it that need to be simplified.
   //
   // Now we will process the contents of 'result',
   // replacing any "." or ".." with the appropriate
   // stuff, and putting the completed path back into
   // 'fn' as we go.
   //

   // Copy the drive.
   fn = origfn;
   rpos = 0;
   if (result[1] == ':')
   {
      // Skip regular "d:" drive
      *fn++ = result[rpos++];
      *fn++ = result[rpos++];
   }
   else if (fn[0] == '\\' && fn[1] == '\\')
   {
      // Skip network "\\machine\drivename"
      *fn++ = result[rpos++];
      *fn++ = result[rpos++];
      while (!is_path_separator(result[rpos]) && result[rpos] != '\0')
         *fn++ = result[rpos++];
      *fn++ = result[rpos++];
      while (!is_path_separator(result[rpos]) && result[rpos] != '\0')
         *fn++ = result[rpos++];
   }
   *fn = '\0';

   // Now process each element in the path.
   while (result[rpos] != '\0')
   {
      // Skip leading path separator (if any).
      while (is_path_separator(result[rpos]))
         rpos++;

      // Extract element from path.
      epos = 0;
      while (!is_path_separator(result[rpos]) && result[rpos] != '\0')
      {
         element[epos++] = result[rpos++];
      }
      element[epos] = '\0';

      // Is this a "." ?
      if (_tcscmp(element, _T(".")) == 0)
      {
         // Ignore 'current directory'
      }
      // Is this a ".." ?
      else if (_tcscmp(element, _T("..")) == 0)
      {
         // Back up one element in 'fn'.
         while (!is_path_separator(*fn))
            fn--;
         *fn = '\0';
      }
      // Normal subdirectory name (or filename).
      else
      {
         _tcscat_s(fn, _tcslen(fn) + 2, _T("\\"));
         _tcscat_s(fn, _tcslen(fn) + _tcslen(element) + 1, element);
         fn = &fn[_tcslen(fn)];
      }
   }
}

//
// FindBaseFilename:
// Given the pathname of a file, finds the filename portion
// (minus the preceeding directory path) and returns a pointer
// to it.  Returns NULL if error.
//
const _TCHAR *
FindBaseFilename(const _TCHAR *pszPath)
{
   if (pszPath == NULL || pszPath[0] == '\0')
      return NULL;
   const _TCHAR *p = _tcsrchr(pszPath, _TCHAR('\\'));
   if (p == NULL)
      return &pszPath[0];
   else if (*p == '\\')
      return (p + 1);
   else
      return p;
}

//
// readline:
// Reads a line of text from an open file.
// Lines longer than MAXLINE characters will be
// split into multiple lines.
//
// Carriage returns are ignored.  Line feeds mark end of line.
// Carriage return and line feed characters are not included
// in the returned string.
//
// Parameters:
//   Name   Description
//   ----   -----------
//   fp     Open file to read from.
//   s      String to be filled.  Result will be null-terminated.
//          This buffer should be at least MAXLINE characters in
//          size.
//   smax   Maximum size of buffer 's'.
//
// Returns:
//   Value   Meaning
//   -----   -------
//   1       Successful.
//   0       Error or end-of-file occured.
//
int
readline(FILE *fp, _TCHAR *s, int smax)
{
   int   pos = 0;    // Current position in string.
   int   ch;         // Character from file.
   int   count = 0;  // # of bytes read from file.

   // Read a maximum of smax characters from the file.
   s[pos] = '\0';
   while (pos < smax - 1)
   {
      // Read a character.
      ch = fgetc(fp);
      if (ch == EOF)
      {
         // Hit the end of file.
         if (count > 0)
            return 1;
         else
            return 0;
      }

      // Update count of characters read from file.
      count++;

      // Handle the character we just read.
      if (ch == '\r')
      {
         // Ignore carriage returns.
         ;
      }
      else if (ch == '\n')
      {
         // End of line.
         return 1;
      }
      else
      {
         // Add character to string.
         s[pos++] = static_cast<_TCHAR>(ch);
         s[pos] = '\0';
      }
   }

   // Success!
   return 1;
}

//
// OptionNameIs:
// Checks the name of a command line option.
//
// Parameters:
//    Name     Description
//    ----     -----------
//    szArg    Command line argument to be examined.
//    szName   Name to compare to command line option string.
//
// Returns true if specified command line option matches the
// specified name; false otherwise.
//
bool
OptionNameIs(const _TCHAR *szArg, const _TCHAR *szName)
{
   // Check for bogus arguments.
   if (szArg == NULL || szArg[0] == '\0' || szName == NULL || szName[0] == '\0')
      return false;

   // If the command line argument is preceeded by a '-' or '/'
   // switch indicator, then skip the first character.
   if (szArg[0] == '-' || szArg[0] == '/')
      szArg++;

   // Compare the specified name with the argument string.
   while (*szArg && *szName)
   {
      // Does this character match?
      if (tolower(*szName++) != tolower(*szArg++))
         return false;
   }

   // If the argument string ended before the name string, then
   // bail.
   if (*szName)
      return false;

   // If the next character of the argument string is alphanumeric,
   // then bail.
   if (isalpha(*szArg) || isdigit(*szArg) || *szArg == '_')
      return false;

   // We have a match.
   return true;
}

//
// OptionValue:
// Retrieves a pointer to the value portion of a command
// line option.  The input string is assumed to be of the
// form OPTIONNAME=OPTIONVALUE
//
// Parameters:
//    Name     Description
//    ----     -----------
//    szArg    Pointer to command line argument to be examined.
//
// Returns pointer to value portion of szArg if successful.
// Returns pointer to empty string if error occurs.
//
const _TCHAR *
OptionValue(const _TCHAR *szArg)
{
   static _TCHAR *empty = _T("");
   static _TCHAR p[1024] = _T("");

   if (szArg == NULL)
      return empty;

   while (*szArg && *szArg != '=' && *szArg != ':')
      szArg++;

   if (*szArg == '=' || *szArg == ':')
   {
      szArg++; // Skip '=' or ':'

      // Remove leading quote (if any).
      if (*szArg == '"' && szArg[1] != '\0')
         szArg++;

      // Copy the rest of the string.
      _tcscpy_s(p, 1024, szArg);

      // Remove trailing spaces (if any).
      while (_tcslen(p) > 0 && p[_tcslen(p) - 1] == ' ' || p[_tcslen(p) - 1] == '\t')
         p[_tcslen(p) - 1] = '\0';

      // Remove trailing quote (if any).
      if (_tcslen(p) > 1 && p[_tcslen(p) - 1] == '"')
         p[_tcslen(p) - 1] = '\0';

      // Caller gets pointer to string buffer.
      return &p[0];
   }

   return empty;
}

//
// FormatThousands:
// Given a number in text format, inserts commas at
// the thousands, millions, billions, etc.  The text
// string pointed to should have enough extra room
// in it for inserting the commas.
//
void
FormatThousands(_TCHAR *pszNumber)
{
   // Firstly, skip anything after the decimal.
   _TCHAR *p = _tcsrchr(pszNumber, _TCHAR('.'));
   if (p == NULL)
      p = &pszNumber[_tcslen(pszNumber) - 1];
   else
      p--;

   // Loop until all digits are processed.
   int iSkip = 0;
   while (p > pszNumber)
   {
      iSkip++;
      if (iSkip == 3)
      {
         memmove(p + 1, p, sizeof(_TCHAR) * (_tcslen(p) + 1));
         *p = ',';
         iSkip = 0;
      }
      p--;
   }
}

