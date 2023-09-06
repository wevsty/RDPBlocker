#include "system_com_utils.h"

SystemComInitialize::SystemComInitialize()
{
    HRESULT hResult = CoInitializeEx(0, COINIT_MULTITHREADED);
    switch (hResult)
    {
        case S_OK:
            break;
        case S_FALSE:
            break;
        case RPC_E_CHANGED_MODE:
            break;
        default:
            std::exit(-1);
    }
}

SystemComInitialize::~SystemComInitialize()
{
    CoUninitialize();
}
