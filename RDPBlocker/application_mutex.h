#ifndef __APPLICATION_MUTEX__
#define __APPLICATION_MUTEX__

#if !defined(NOMINMAX)
#define NOMINMAX 1
#endif

#include <windows.h>
#include <string>

#include <boost/locale.hpp>

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
