#include "pe_checksum.h"

bool check_pe_checksum(const std::wstring& file_path)
{
#ifdef _DEBUG
    assert(file_path.empty() == false);
    return true;
#else
    DWORD header_checksum = 0;
    DWORD current_checksum = 0;
    DWORD status = MapFileAndCheckSumW(file_path.c_str(), &header_checksum,
                                       &current_checksum);
    if (status != CHECKSUM_SUCCESS)
    {
        return false;
    }
    if (header_checksum != current_checksum)
    {
        return false;
    }
    return true;
#endif  // _DEBUG
}
