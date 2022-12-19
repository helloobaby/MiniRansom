//https://en.cppreference.com/w/cpp/filesystem/path/extension
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

int main() {
  fs::path p(
      L"C:\\Users\\Admin\\AppData\\Local\\Microsoft\\OneDrive\\21.139.0711."
      L"0001\\api-ms-win-crt-stdio-l1-1-0.dll");

  printf("%ws\n", p.extension().c_str());
  return 0;
}
