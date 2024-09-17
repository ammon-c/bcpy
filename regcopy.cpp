//--------------------------------------------------------------------
//
// regcopy.cpp
//
// C++ code for REGCOPY utility.  Copies the contents of the Windows
// registry to a file.
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

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <direct.h>
#include <time.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

//----------------------------------------------------------
// MACROS
//----------------------------------------------------------

// Define the symbol DBG to enable debug/trace output to console.
#define DBG

//----------------------------------------------------------
// FUNCTIONS
//----------------------------------------------------------

//
// errmsg:
// Outputs error messages to the console in a consistent format.
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
errmsg(const char *pszFile, int iLine, const char *msg, const char *omsg = NULL)
{
   char szText[MAX_PATH * 5];

   if (msg == NULL)
      return;
   sprintf_s(szText, MAX_PATH * 5, "regcopy|%s(%d):  %s", pszFile == NULL ? "unknown" : pszFile, iLine < 1 ? 0 : iLine, msg);
   if (omsg != NULL && omsg[0] != '\0')
   {
      strcat_s(szText, MAX_PATH * 5, ":  ");
      strcat_s(szText, MAX_PATH * 5, omsg);
   }
   strcat_s(szText, MAX_PATH * 5, "\n");
   printf("%s", szText);
   fflush(stdout);
}

//
// WinErrMsg:
// Outputs a message for a Windows system error code.
//
static void
WinErrMsg(LONG lCode)
{
   printf("  Windows error code %ld.\n", lCode);

   // Format error code from windows.
   char szBuffer[MAX_PATH * 3];
   DWORD dwResult = FormatMessageA(
                     FORMAT_MESSAGE_FROM_SYSTEM,
                     NULL,             // lpSource
                     lCode,            // dwMessageID
                     0,                // dwLanguageID
                     szBuffer,         // lpBuffer
                     MAX_PATH * 3 -1,  // nSize
                     NULL              // vaargs
                     );
   (void)dwResult;

   printf("  Windows error:  %s\n", szBuffer);
}

//
// DoSaveRegistry:
// Saves the registry to the specified file.
// Returns true if successful, false if error.
//
static bool
DoSaveRegistry(const char *pszOutFile)
{
   LONG lResult = 0; // Used below to store error code.

   // Delete the specified output file, because RegSaveKey will
   // fail if the file already exists.
   _unlink(pszOutFile);

   //
   // Allow this process to have the "backup" privilege.
   // This is required or RegSaveKey will fail.
   //
   TOKEN_PRIVILEGES tp;
   HANDLE hToken;
   LUID luid;
   if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
   {
      errmsg(__FILE__, __LINE__, "OpenProcessToken failed");
      WinErrMsg(GetLastError());
      return false;
   }
   if (!LookupPrivilegeValue(NULL, SE_BACKUP_NAME, &luid))
   {
      errmsg(__FILE__, __LINE__, "LookupPrivilegeValue failed");
      WinErrMsg(GetLastError());
      return false;
   }
   memset(&tp, 0, sizeof(tp));
   tp.PrivilegeCount = 1;
   tp.Privileges[0].Luid = luid;
   tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
   if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
   {
      errmsg(__FILE__, __LINE__, "AdjustTokenPrivileges failed");
      WinErrMsg(GetLastError());
      return false;
   }

   // Save registry to specified file.
   lResult = RegSaveKeyA(HKEY_CLASSES_ROOT, pszOutFile, NULL);
   if (lResult != ERROR_SUCCESS)
   {
      // Tell the user an error occurred.
      // Probably ERROR_PRIVILEGE_NOT_HELD
      errmsg(__FILE__, __LINE__, "RegSaveKey failed");
      WinErrMsg(lResult);

      // Delete the incomplete registry file.
      _unlink(pszOutFile);

      return false;
   }

   // Revoke any extra privileges this process holds.
   AdjustTokenPrivileges(hToken, TRUE, NULL, 0, NULL, NULL);

   return true;
}

//
// main:
// Application entry point.  Uses standard arguments
// and return values for a command-line program.
//
int
main(int argc, char **argv)
{
   // Sign on.
   const char *pszSignon = "REGCOPY Version 1.01 (C) Copyright 1995-2005 A.R.Campbell\n";
   printf("%s\n", pszSignon);

   // See if user needs help.
   if (argc != 2)
   {
      printf("Usage:  regcopy outfile.dat\n");
      return EXIT_FAILURE;
   }
   const char *pszOutFile = argv[1];

   // Open a copy of the registry root key.
   HKEY hRoot = NULL;
   LONG lResult = RegOpenKeyEx(HKEY_CLASSES_ROOT, NULL, 0, KEY_READ, &hRoot);
   if (lResult != ERROR_SUCCESS)
   {
      // Tell the user an error occurred.
      errmsg(__FILE__, __LINE__, "RegOpenKey failed");
      WinErrMsg(lResult);
      return EXIT_FAILURE;
   }


   // Save the registry.
   printf("Saving registry to file:  %s\n", pszOutFile);
   if (!DoSaveRegistry(pszOutFile))
   {
      _unlink(pszOutFile);
      return EXIT_FAILURE;
   }

   RegCloseKey(hRoot);

   printf("Done.\n");
   return EXIT_SUCCESS;
}

