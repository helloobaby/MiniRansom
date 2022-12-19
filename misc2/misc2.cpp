// 拒绝访问
// TrustInstaller

#include <iostream>
#include <Windows.h>

#include "..\method1\utils.h"

int main() {
  std::locale::global(std::locale(""));
  bool b = MoveFileW(
      L"C:\\Users\\123\\Desktop\\Dbgview.exe",
      L"C:\\Users\\123\\Desktop\\Dbgview.exe.encrypt");
  if (!b) {
    DisplayError(GetLastError());
    return -1;
  }
  return 0;
}
