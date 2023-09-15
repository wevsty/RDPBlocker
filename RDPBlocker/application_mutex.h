#ifndef __APPLICATION_MUTEX__
#define __APPLICATION_MUTEX__

#include <windows.h>
#include <string>

#include "utf_convert.h"

class ApplicationMutex
{
    private:
    HANDLE m_handle;
    bool create(const std::string& mutex_name);
    bool open(const std::string& mutex_name);
    void close();
    bool is_exist(const std::string& mutex_name);

    public:
    ApplicationMutex();
    ~ApplicationMutex();
    bool lock(const std::string& mutex_name);
    void unlock();
};

#endif  //__APPLICATION_MUTEX__
