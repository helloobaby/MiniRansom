#include <filesystem>
#include <fstream>

#include <Windows.h>
#include <strsafe.h>
#include <locale.h>
#include <algorithm>

__inline 
VOID DisplayError(_In_ DWORD Code)
{
  _Null_terminated_ WCHAR buffer[MAX_PATH] = {0};
  DWORD count;
  HMODULE module = NULL;
  HRESULT status;

  count = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, Code, 0, buffer,
                        sizeof(buffer) / sizeof(WCHAR), NULL);

  if (count == 0) {
    count = GetSystemDirectory(buffer, sizeof(buffer) / sizeof(WCHAR));

    if (count == 0 || count > sizeof(buffer) / sizeof(WCHAR)) {
      //
      //  In practice we expect buffer to be large enough to hold the
      //  system directory path.
      //

      printf("    Could not translate error: %u\n", Code);
      return;
    }

    status =
        StringCchCat(buffer, sizeof(buffer) / sizeof(WCHAR), L"\\fltlib.dll");

    if (status != S_OK) {
      printf("    Could not translate error: %u\n", Code);
      return;
    }

    module = LoadLibraryExW(buffer, NULL, LOAD_LIBRARY_AS_DATAFILE);

    //
    //  Translate the Win32 error code into a useful message.
    //

    count = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE, module, Code, 0, buffer,
                          sizeof(buffer) / sizeof(WCHAR), NULL);

    if (module != NULL) {
      FreeLibrary(module);
    }

    //
    //  If we still couldn't resolve the message, generate a string
    //

    if (count == 0) {
      printf("    Could not translate error: %u\n", Code);
      return;
    }
  }

  //
  //  Display the translated error.
  //

  printf("    %ws\n", buffer);
}

__inline
BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid))
    {
        return FALSE;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    if (bEnablePrivilege)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
    {
        return FALSE;
    }

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
    {
        return FALSE;
    }

    return TRUE;
}

__inline
bool findStringIC(const std::string& strHaystack, const std::string& strNeedle)
{
    auto it = std::search(
        strHaystack.begin(), strHaystack.end(),
        strNeedle.begin(), strNeedle.end(),
        [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
    );
    return (it != strHaystack.end());
}

__inline
bool findStringIC(const std::wstring& strHaystack, const std::wstring& strNeedle)
{
    auto it = std::search(
        strHaystack.begin(), strHaystack.end(),
        strNeedle.begin(), strNeedle.end(),
        [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
    );
    return (it != strHaystack.end());
}

