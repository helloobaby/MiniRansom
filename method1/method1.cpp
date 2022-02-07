#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<windows.h>
#include<vector>
#include<thread>

#include"find_file.h"

using namespace std;


//记录磁盘数
uint32_t harddriver_count;
//存储盘名  ，如 "C:\" "D:\"
vector<wstring> harddriver_name;

int main()
{
    WCHAR rootPath[10] = { 0 }, driveType[MAX_PATH] = { 0 };
    UINT nType;

    std::wcout.imbue(std::locale("chs"));

    for (char a = 'A'; a <= 'Z'; a++)
    {
        wsprintf(rootPath, L"%c:\\", a);
        nType = GetDriveTypeW(rootPath);
        if (nType != DRIVE_NO_ROOT_DIR)                  // DRIVE_NO_ROOT_DIR: 路径无效  
        {
            if (nType == DRIVE_FIXED)
            {
                harddriver_count++;
                harddriver_name.push_back(rootPath);
            }
        }
    }

#ifdef _DEBUG
    for (auto driver : harddriver_name)
        wcout << driver.c_str() << endl;
#endif

    vector<wstring>::const_iterator begin = harddriver_name.cbegin();
    while (begin != harddriver_name.cend()) {
        
        thread t1(find_file_recursive,*begin, true);
        t1.detach();

        begin++;
    }














    getchar();
    return 0;
}

