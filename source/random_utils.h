#ifndef __RANDOM_UTILS__
#define __RANDOM_UTILS__

#include <random>
#include <string>

int random_int(const int min, const int max);

std::string random_string(
    const unsigned int count,
    const std::string& chars_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

#endif  // __STD_RANDOM_UTILS__
