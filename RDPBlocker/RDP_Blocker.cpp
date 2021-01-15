#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

// Set Windows SDK Version
#include <SDKDDKVer.h>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/thread.hpp>

#include "application_mutex.h"
#include "block_address_status.h"
#include "boost_xml_utils.h"
#include "expires_map_utils.h"
#include "firewall_controller.h"
#include "logger.h"
#include "PECheckSum.h"
#include "program_path_utils.h"
#include "self_exit_code.h"
#include "std_random_utils.h"
#include "system_com_utils.h"
#include "system_event_log.h"


namespace program_setting {
    const std::string app_mutex_name = "RDPBlocker-locker";
    const std::string program_version = "1.1.2.0";
    const std::string rule_prefix = "AUTO_BLOCKED_";
    std::string config_file_path;
    std::vector<boost::regex> whitelist;
    // 阻挡阈值
    int block_threshold;
    // 阻挡时间
    int block_time;
}


class AutoBlockRemoteAddress : public std::enable_shared_from_this<AutoBlockRemoteAddress>
{
    boost::asio::io_context& m_io_context;
    boost::asio::steady_timer m_block_timer;
    boost::asio::steady_timer m_unblock_timer;

public:
    AutoBlockRemoteAddress(boost::asio::io_context& io_context)
        : m_io_context(io_context), m_block_timer(io_context), m_unblock_timer(io_context)
    {
        m_block_timer.expires_at(boost::asio::steady_timer::time_point::max());
        m_unblock_timer.expires_at(boost::asio::steady_timer::time_point::max());
    }

    ~AutoBlockRemoteAddress()
    {

    }

    void Block(
        const std::string& rule_name,
        const std::string& block_address
    )
    {
        g_logger->info("Add rule {}", rule_name);
        firewall_controller::BlockInboundIP(rule_name, block_address);
    }

    void Unblock(
        const std::string& rule_name
    )
    {
        g_logger->info("Delete rule {}", rule_name);
        firewall_controller::DeleteRuleByName(rule_name);
    }

    void AsyncBlock(
        const std::string& rule_name,
        const std::string& block_address
    )
    {
        m_io_context.post(
            boost::bind(&AutoBlockRemoteAddress::Block, shared_from_this(), rule_name, block_address)
        );
    }

    void AsyncUnblock(
        const std::string& rule_name,
        const int delay_time
    )
    {
        m_unblock_timer.expires_from_now(std::chrono::seconds(delay_time));
        m_unblock_timer.async_wait(
            boost::bind(&AutoBlockRemoteAddress::Unblock, shared_from_this(), rule_name)
        );
    }
};

void CreateBlockWorkPlan(
    boost::asio::io_context& io_context,
    const std::string& address,
    const int block_time
)
{
    // 生成随机的规则名
    enum {
        RULE_RANDOM_CHARS_LENGTH = 8
    };
    std::string rule_name = program_setting::rule_prefix;
    rule_name += random_string(RULE_RANDOM_CHARS_LENGTH);

    std::shared_ptr<AutoBlockRemoteAddress> blocker_ptr = 
        std::make_shared<AutoBlockRemoteAddress>(io_context);
    // 添加阻止规则
    blocker_ptr->AsyncBlock(rule_name, address);

    // 超时自动删除规则
    blocker_ptr->AsyncUnblock(rule_name, block_time);
}

// 验证IP合法性
bool IsIPAddress(const std::string& data) 
{
    try
    {
        boost::asio::ip::address addr = boost::asio::ip::address::from_string(data);
        return true;
    }
    catch (std::exception& err)
    {
        std::cout << err.what() << std::endl;
        return false;
    }
}

// 验证IP是否为白名单
bool IsWhitelistAddress(const std::string& data)
{
    for (const auto& expr : program_setting::whitelist)
    {
        if (regex_find_match(expr, data) == true)
        {
            return true;
        }
    }
    return false;
}

void ProcessRemoteAddresses(
    boost::asio::io_context& io_context,
    std::map<std::string, block_address_status>& address_count,
    const std::string& address
)
{
    // 校验IP地址
    if (IsIPAddress(address) ==  false)
    {
        g_logger->info("Invalid address : {}", address);
        return;
    }
    if (IsWhitelistAddress(address) == true)
    {
        g_logger->info("Whitelist address : {}", address);
        return;
    }
    auto find_iter = address_count.find(address);
    if (find_iter == address_count.end())
    {
        block_address_status status;
        status.set_count(1);
        status.set_expire_interval(program_setting::block_time);
        address_count[address] = status;
        find_iter = address_count.find(address);
    }
    else
    {
        int n_count = find_iter->second.get_count() + 1;
        find_iter->second.set_count(n_count);
        find_iter->second.set_expire_interval(program_setting::block_time);
    }
    if (find_iter->second.get_count() >= program_setting::block_threshold)
    {
        if (find_iter->second.is_blocked() == false)
        {
            g_logger->info("Block address : {}", address);
            CreateBlockWorkPlan(io_context, address, program_setting::block_time);
            find_iter->second.set_blocked_interval(program_setting::block_time);
            find_iter->second.set_expire_interval(program_setting::block_time);
        }
    }
    delete_expire_keys(address_count);
}

void SubscribeSystemAuthEvent(boost::asio::io_context* io_context_ptr)
{
    boost::asio::io_context& io_context = *io_context_ptr;
    std::map<std::string, block_address_status> address_count;
    // g_logger->info("Subscribe RDP auth failed event.");
    g_logger->info("Subscribe RDP event.");
    SubscribeSystemEvent os_event_log;
    // 订阅RDP登录失败事件
    if (os_event_log.SubscribeRDPAuthFailedEvent() != true)
    {
        g_logger->error("SubscribeRDPAuthFailedEvent failed.");
        std::exit(EXIT_CODE::SUBSCRIBE_EVENT_ERROR);
    }
    while (true)
    {
        g_logger->debug("Wait event.");
        std::vector<std::string> vt_buffer;
        // 等待事件信号
        if (os_event_log.WaitSignal() != true)
        {
            break;
        }
        // 保存订阅的日志信息
        os_event_log.StoreEventLogResults(vt_buffer);
        // 复位信号
        os_event_log.ResetSignal();

        // 处理暂存的信息
        for (auto& xml_data : vt_buffer)
        {
            std::map<std::string, std::string> event_attr;
            GetLogEventDataToMap(xml_data, event_attr);
            std::string address = event_attr["IpAddress"];
            //std::cout << address << std::endl;
            if (address != "-")
            {
                g_logger->info("Address auth failed : {}", address);
                ProcessRemoteAddresses(io_context, address_count, address);
            }
        }
        
    }
}

// 载入配置文件
bool load_config_file(const std::string& file_path)
{
    bool bRet = true;
    try
    {
        boost::property_tree::ptree pt;
        boost::property_tree::ini_parser::read_ini(file_path, pt);

        program_setting::block_threshold = pt.get<int>("Block.threshold");
        program_setting::block_time = pt.get<int>("Block.time");

    
        boost::property_tree::ptree whitelist_pt = pt.get_child("Whitelist");
        for (auto& it : whitelist_pt)
        {
            boost::regex regex_obj(it.second.data());
            program_setting::whitelist.push_back(regex_obj);
        }
    }
    catch (std::exception& err)
    {
        bRet = false;
        std::cout << "Loading configuration file error." << std::endl;
        std::cout << err.what() << std::endl;
    }
    return bRet;
}

// 确保程序目录为工作目录
void ensure_self_work_dir()
{
    std::string work_dir = get_self_dir_path();
    set_work_dir(work_dir);
}

// 对程序自身进行校验
void check_self_file()
{
    std::string self_path = get_self_file_path();
    std::wstring ws_path = boost::locale::conv::utf_to_utf<wchar_t>(self_path);
    bool status = PECheckSum(ws_path);
    if (status == false)
    {
        std::exit(EXIT_CODE::CHECKSUM_ERROR);
    }
}

// 解析命令行参数
void prase_argv(int argc, char* argv[])
{
    try
    {
        // namespace BPO = boost::program_options;
        boost::program_options::options_description options("RDPBlocker Options");
        options.add_options()
            ("help", "Produce help message")
            ("config", boost::program_options::value<std::string>(&program_setting::config_file_path), "file path");
        // parse program options
        boost::program_options::variables_map var_map;
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, options), var_map);
        boost::program_options::notify(var_map);

        if (var_map.count("help"))
        {
            std::cout << options << std::endl;
            std::exit(EXIT_CODE::SUCCESS);
        }

        if (var_map.count("config") == 0)
        {
            // 如果未设定加载的配置文件默认为 config.ini
            program_setting::config_file_path = "config.ini";
        }
        if (load_config_file(program_setting::config_file_path) == false)
        {
            std::exit(EXIT_CODE::LOAD_CONFIG_ERROR);
        }
    }
    catch (std::exception& err)
    {
        std::cout << "Parsing command line error" << std::endl;
        std::cout << err.what() << std::endl;
        std::exit(EXIT_CODE::COMMAND_PARSE_ERROR);
    }
}

int main(int argc, char* argv[])
{
    // 确保程序目录为工作目录
    ensure_self_work_dir();

    // 对程序文件进行自校验
    check_self_file();

    // 解析命令行参数
    prase_argv(argc, argv);

    // 初始化logger
    init_logger();
    g_logger->info("RDPBlocker Version {}", program_setting::program_version);

    // 确保系统中只有一个RDPBlocker运行，以免互相干扰。
    ApplicationMutex app_mutex;
    if (app_mutex.Lock(program_setting::app_mutex_name) == false)
    {
        g_logger->warn("RDPBlocker already running in the system");
        g_logger->warn("Please do not start more than one process at the same time");
        return EXIT_CODE::APP_EXIST_ERROR;
    }

    // 初始化COM
    g_logger->debug("Initialization COM");
    SystemComInitialize sys_com_init;

    // 重启时自动删除遗留规则
    g_logger->debug("Remove existing auto-blocking rules");
    firewall_controller::DeleteRulesByRegex(program_setting::rule_prefix);

    boost::asio::io_context io_context;
    
    // 创建一个空任务 防止io_context.run()立刻返回
    boost::asio::io_context::work idle_work(io_context);
    
    // 启动IO线程
    boost::thread io_thread(
        boost::bind(&boost::asio::io_context::run, &io_context)
    );

    // 启动事件订阅线程
    boost::thread event_thread(
        boost::bind(&SubscribeSystemAuthEvent, &io_context)
    );
    
    event_thread.join();
    io_thread.join();
    return EXIT_CODE::SUCCESS;
}
