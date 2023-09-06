#include "application_mutex.h"

bool ApplicationMutex::Create(const std::string& mutex_name)
{
    std::wstring ws_mutex_name =
        boost::locale::conv::to_utf<WCHAR>(mutex_name, "UTF-8");
    m_handle = CreateMutexW(NULL, FALSE, ws_mutex_name.c_str());
    if (m_handle == NULL)
    {
        return false;
    }
    return true;
}

bool ApplicationMutex::Open(const std::string& mutex_name)
{
    std::wstring ws_mutex_name =
        boost::locale::conv::to_utf<WCHAR>(mutex_name, "UTF-8");
    m_handle = OpenMutexW(MUTEX_ALL_ACCESS, FALSE, ws_mutex_name.c_str());
    if (m_handle == NULL)
    {
        return false;
    }
    return true;
}

void ApplicationMutex::Close()
{
    if (m_handle != NULL)
    {
        CloseHandle(m_handle);
        m_handle = NULL;
    }
}

bool ApplicationMutex::IsExist(const std::string& mutex_name)
{
    if (m_handle != NULL)
    {
        return true;
    }
    bool bStatus = Open(mutex_name);
    if (bStatus == false)
    {
        DWORD dwCode = GetLastError();
        if (dwCode == ERROR_FILE_NOT_FOUND)
        {
            return false;
        }
    }
    return true;
}

ApplicationMutex::ApplicationMutex() : m_handle(NULL)
{
}

ApplicationMutex::~ApplicationMutex()
{
    Close();
}

bool ApplicationMutex::Lock(const std::string& mutex_name)
{
    if (IsExist(mutex_name) == true)
    {
        return false;
    }
    bool bRet = Create(mutex_name);
    return bRet;
}

void ApplicationMutex::Unlock()
{
    Close();
}
