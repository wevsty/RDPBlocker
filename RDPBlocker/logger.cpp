#include "logger.h"

std::shared_ptr<spdlog::logger> g_logger = NULL;

logger_options::logger_options() : level(spdlog::level::debug)
{
}

logger_options::~logger_options()
{
}

void logger_options::set_level_string(const std::string& level_string)
{
    // 转化为小写
    std::string lower_level = level_string;
    std::for_each(lower_level.begin(), lower_level.end(),
                  [](char& c)
                  {
                      c = static_cast<char>(std::tolower(c));
                  });
    level = spdlog::level::from_str(lower_level);
}

std::string logger_options::get_level_string() const
{
    return std::string(spdlog::level::to_short_c_str(level));
}
