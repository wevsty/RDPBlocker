#ifndef __PE_FILE_CHECKSUM__
#define __PE_FILE_CHECKSUM__

#include <cassert>
#include <string>

#include <windows.h>

#include <imagehlp.h>

#pragma comment(lib, "imagehlp.lib")

bool check_pe_checksum(const std::wstring& file_path);

#endif  //__PE_FILE_CHECKSUM__
