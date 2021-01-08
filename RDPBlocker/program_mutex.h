#ifndef __PROGRAM_MUTEX__
#define __PROGRAM_MUTEX__

#include <string>
#include <windows.h>

#include <boost/locale.hpp>

#include "handle_wrapper.h"

bool LockAppMutex(HANDLE& hMutex, const std::string& mutex_name);
void UnLockAppMutex(HANDLE& hMutex);

#endif //__PROGRAM_MUTEX__
