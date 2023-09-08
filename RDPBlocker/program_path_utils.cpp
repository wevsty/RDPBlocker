#include "program_path_utils.h"

std::string self_file_path()
{
    std::string program_path;
    std::shared_ptr<WCHAR[]> path_buffer =
        std::make_shared<WCHAR[]>(SYSTEM_MAX_PATH);
    ZeroMemory(path_buffer.get(), SYSTEM_MAX_PATH);

    WCHAR* lpFileName = path_buffer.get();
    DWORD error_code = ERROR_SUCCESS;
    SetLastError(error_code);
    GetModuleFileNameW(NULL, lpFileName, SYSTEM_MAX_PATH);
    error_code = GetLastError();
    assert(error_code == ERROR_SUCCESS);
    program_path = utf_to_utf<char>(lpFileName);
    return program_path;
}

std::string self_directory_path()
{
    boost::filesystem::path program_path = self_file_path();
    return program_path.parent_path().string();
}

std::string get_work_dir()
{
    std::string work_dir;
    std::shared_ptr<WCHAR[]> path_buffer =
        std::make_shared<WCHAR[]>(SYSTEM_MAX_PATH);
    ZeroMemory(path_buffer.get(), SYSTEM_MAX_PATH);

    WCHAR* szCurrentDirectory = path_buffer.get();
    DWORD status = GetCurrentDirectoryW(SYSTEM_MAX_PATH, szCurrentDirectory);
    if (status == 0)
    {
        return work_dir;
    }
    work_dir = utf_to_utf<char>(szCurrentDirectory);
    return work_dir;
}

bool set_work_dir(const std::string& path)
{
    std::wstring work_dir = utf_to_utf<WCHAR>(path);
    BOOL bRet = SetCurrentDirectoryW(work_dir.c_str());
    if (bRet == 0)
    {
        return false;
    }
    return true;
}
