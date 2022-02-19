#include <iostream>
#include <windows.h>

using namespace std;

int main()
{
    DWORD lastError;
    WCHAR driveletter[] = L"\\\\.\\C:\\$Directory";
    HANDLE hVol = CreateFile(driveletter, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    if (hVol == INVALID_HANDLE_VALUE)
    {
        lastError = GetLastError();
    }

    return 0;
}

