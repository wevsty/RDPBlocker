#include "random_utils.h"

int random_int(const int min, const int max)
{
    std::random_device rd;
    std::default_random_engine engine(rd());
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(engine);
}

std::string random_string(const unsigned int count,
                          const std::string& chars_table)
{
    std::string buffer;
    if (chars_table.empty())
    {
        return buffer;
    }
    std::size_t max_table_index = chars_table.size() - 1;
    std::random_device rd;
    std::default_random_engine engine(rd());
    std::uniform_int_distribution<> distrib(0,
                                            static_cast<int>(max_table_index));
    if (count > 0)
    {
        buffer.resize(count);
    }
    for (unsigned int i = 0; i < count; ++i)
    {
        buffer[i] = chars_table[distrib(engine)];
    }
    return buffer;
}
