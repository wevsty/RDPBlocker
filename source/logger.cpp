#include "logger.h"

// 全局logger
std::shared_ptr<spdlog::logger> g_logger = spdlog::default_logger();

// 初始化全局logger
void initialize_global_logger(std::shared_ptr<spdlog::logger>& logger)
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

        logger = shared_logger;
    }
    catch (const spdlog::spdlog_ex& err)
    {
        std::cout << "Loger init failed: " << err.what() << std::endl;
        std::exit(APPLICATION_EXIT_CODE::FAILED);
    }
}
