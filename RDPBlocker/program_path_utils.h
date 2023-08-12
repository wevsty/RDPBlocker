#ifndef __PROGRAM_PATH_UTILS__
#define __PROGRAM_PATH_UTILS__

#include <string>
#include <memory>

#include <boost/locale.hpp>
#include <boost/filesystem.hpp>

#include <windows.h>

#define OS_MAX_PATH 32768

// 获取程序自身路径
std::string get_self_file_path();

// 获取程序自身所在的文件夹路径
std::string get_self_dir_path();

// 获取工作目录
std::string get_work_dir();

// 设置工作目录
bool set_work_dir(const std::string& path);


#endif // __PROGRAM_PATH_UTILS__