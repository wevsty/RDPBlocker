#include "program_path_utils.h"

std::string get_self_file_path()
{
    std::string program_path;
    WCHAR lpFileName[OS_MAX_PATH];
    int nSize = ::GetModuleFileNameW(NULL, lpFileName, OS_MAX_PATH);

    program_path = boost::locale::conv::utf_to_utf<char>(lpFileName);
    return program_path;
}

std::string get_self_dir_path()
{
    boost::filesystem::path program_path = get_self_file_path();
    return program_path.parent_path().string();
}

std::string get_work_dir()
{
    std::string work_dir;
    WCHAR szCurrentDirectory[OS_MAX_PATH];
    DWORD status = GetCurrentDirectoryW(OS_MAX_PATH, szCurrentDirectory);
    if (status == 0)
    {
        return work_dir;
    }
    work_dir = boost::locale::conv::utf_to_utf<char>(szCurrentDirectory);
    return work_dir;
}

bool set_work_dir(const std::string& path)
{
    std::wstring work_dir = boost::locale::conv::to_utf<WCHAR>(path, "UTF-8");
    BOOL bRet = SetCurrentDirectoryW(work_dir.c_str());
    if (bRet == 0)
    {
        return false;
    }
    return true;
}
