#ifndef __YAML_SUPPORT__
#define __YAML_SUPPORT__

#define YAML_CPP_STATIC_DEFINE 1

#include "yaml-cpp/yaml.h"

#if defined(_DEBUG)
#pragma comment(lib, "yaml-cppd.lib")
#else
#pragma comment(lib, "yaml-cpp.lib")
#endif

#endif