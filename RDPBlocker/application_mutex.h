#ifndef __APPLICATION_MUTEX__
#define __APPLICATION_MUTEX__

#include <windows.h>
#include <string>

#include "utf_convert.h"

class ApplicationMutex
{
    private:
    HANDLE m_handle;
    bool Create(const std::string& mutex_name);
    bool Open(const std::string& mutex_name);
    void Close();
    bool IsExist(const std::string& mutex_name);

    public:
    ApplicationMutex();
    ~ApplicationMutex();
    bool Lock(const std::string& mutex_name);
    void Unlock();
};

#endif  //__APPLICATION_MUTEX__
