#include "application_mutex.h"

bool ApplicationMutex::create(const std::string& mutex_name)
{
    std::wstring ws_mutex_name = utf_to_utf<WCHAR>(mutex_name);
    m_handle = CreateMutexW(NULL, FALSE, ws_mutex_name.c_str());
    if (m_handle == NULL)
    {
        return false;
    }
    return true;
}

bool ApplicationMutex::open(const std::string& mutex_name)
{
    std::wstring ws_mutex_name = utf_to_utf<WCHAR>(mutex_name);
    m_handle = OpenMutexW(MUTEX_ALL_ACCESS, FALSE, ws_mutex_name.c_str());
    if (m_handle == NULL)
    {
        return false;
    }
    return true;
}

void ApplicationMutex::close()
{
    if (m_handle != NULL)
    {
        CloseHandle(m_handle);
        m_handle = NULL;
    }
}

bool ApplicationMutex::is_exist(const std::string& mutex_name)
{
    if (m_handle != NULL)
    {
        return true;
    }
    bool bStatus = open(mutex_name);
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
    close();
}

bool ApplicationMutex::lock(const std::string& mutex_name)
{
    if (is_exist(mutex_name) == true)
    {
        return false;
    }
    bool bRet = create(mutex_name);
    return bRet;
}

void ApplicationMutex::unlock()
{
    close();
}
