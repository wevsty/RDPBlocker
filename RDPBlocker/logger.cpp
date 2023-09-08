#include "logger.h"

std::shared_ptr<spdlog::logger> g_logger = spdlog::default_logger();

LoggerConfig::LoggerConfig()
    : m_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v"), m_level(spdlog::level::debug)
{
}

LoggerConfig::~LoggerConfig()
{
}

void LoggerConfig::set_level(const std::string& level_string)
{
    // 转化为小写
    std::string lower_level = level_string;
    std::for_each(lower_level.begin(), lower_level.end(),
                  [](char& c)
                  {
                      c = static_cast<char>(std::tolower(c));
                  });
    m_level = spdlog::level::from_str(lower_level);
}

std::string LoggerConfig::get_level() const
{
    return std::string(spdlog::level::to_short_c_str(m_level));
}

void LoggerConfig::apply()
{
    // g_logger->set_pattern(m_pattern);
    g_logger->set_level(m_level);
}

// 初始化全局logger
void initialize_logger()
{
    try
    {
        // 控制台输出
        auto console_sink =
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        // 创建logger
        std::shared_ptr<spdlog::logger> shared_logger =
            std::make_shared<spdlog::logger>(
                spdlog::logger("main_logger", {console_sink}));

        // 设置输出
        shared_logger->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");
        shared_logger->set_level(spdlog::level::debug);

        // info 自动刷新
        spdlog::flush_on(spdlog::level::info);
        // 每隔1秒自动刷新日志
        spdlog::flush_every(std::chrono::seconds(10));

        // 设置默认logger
        spdlog::set_default_logger(shared_logger);

        g_logger = shared_logger;
    }
    catch (const spdlog::spdlog_ex& err)
    {
        std::cout << "Loger init failed: " << err.what() << std::endl;
        std::exit(APPLICATION_EXIT_CODE::FAILED);
    }
}
