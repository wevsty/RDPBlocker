#include "program_mutex.h"


bool LockAppMutex(HANDLE& hMutex, const std::string& mutex_name)
{
    std::wstring ws_mutex_name = boost::locale::conv::to_utf<WCHAR>(mutex_name, "UTF-8");
    hMutex = CreateMutexW(NULL, FALSE, ws_mutex_name.c_str());
    //如果已经存在同名的Mutex会得到 ERROR_ALREADY_EXISTS
    if (GetLastError() == ERROR_ALREADY_EXISTS) { 
        CloseHandle(hMutex);
        hMutex = NULL;
        return false;
    }
    return true;
}

void UnLockAppMutex(HANDLE& hMutex)
{
    if (hMutex != NULL)
    {
        CloseHandle(hMutex);
        hMutex = NULL;
    }
}
