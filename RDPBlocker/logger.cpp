#include "logger.h"

std::shared_ptr<spdlog::logger> g_logger = NULL;

logger_options::logger_options() 
    : filename("logger.txt"), max_size(1024*1024), max_files(1), level(spdlog::level::debug)
{
}

logger_options::~logger_options()
{
}

void logger_options::set_level_string(const std::string& level_string)
{
    // 转化为小写
    std::string lower_level = level_string;
    std::for_each(lower_level.begin(), lower_level.end(), [](char& c) { c = std::tolower(c); });
    level = spdlog::level::from_str(lower_level);
}

std::string logger_options::get_level_string() const
{
    return std::string(spdlog::level::to_short_c_str(level));
}

void init_global_logger(const logger_options& options)
{
    try
    {
        // 控制台输出
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        // 文件输出
        auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            options.filename, options.max_size, options.max_files
            );

        // spdlog::logger logger("global_logger", { console_sink, rotating_sink });
        auto logger_ptr = std::make_shared<spdlog::logger>(
            spdlog::logger("global_logger", { console_sink, rotating_sink })
            );

        logger_ptr->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");
        logger_ptr->set_level(options.level);
        spdlog::set_default_logger(logger_ptr);
        spdlog::flush_on(spdlog::level::info);
        // 每隔10秒自动刷新日志
        spdlog::flush_every(std::chrono::seconds(10));

        g_logger = logger_ptr;
    }
    catch (const spdlog::spdlog_ex& err)
    {
        std::cout << "Loger init failed: " << err.what() << std::endl;
        std::exit(EXIT_CODE::INIT_LOGGER_ERROR);
    }
}
