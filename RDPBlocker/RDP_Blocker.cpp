#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "logger.h"
#include "block_address_status.h"
#include "boost_xml_utils.h"
#include "expires_map_utils.h"
#include "firewall_controller.h"
#include "program_path_utils.h"
#include "program_mutex.h"
#include "std_random_utils.h"
#include "system_com_utils.h"
#include "system_event_log.h"

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace program_setting {
    const std::string program_version = "1.1";
    const std::string rule_prefix = "AUTO_BLOCKED_";
    std::string config_file_path;
    std::vector<boost::regex> whitelist;
    // 阻挡阈值
    int block_threshold;
    // 阻挡时间
    int block_time;
}

void UnblockRemoteAddresses(
    std::shared_ptr<boost::asio::deadline_timer> unblock_timer_ptr,
    const std::string& rule_name
)
{
    g_logger->info("Delete rule {}", rule_name);
    firewall_controller::DeleteRuleByName(rule_name);
}

void BlockRemoteAddresses(
    const std::string& rule_name,
    const std::string& block_address
)
{
    g_logger->info("Add rule {}", rule_name);
    firewall_controller::BlockInboundIP(rule_name, block_address);
}

void PlanBlockRemoteAddresses(
    boost::asio::io_context& io_context,
    const std::string& address,
    const int block_time
)
{
    // 生成随机的规则名
    enum {
        RULE_RANDOM_CHARS_LENGTH = 6
    };
    std::string rule_name = program_setting::rule_prefix;
    rule_name += random_string(RULE_RANDOM_CHARS_LENGTH);

    // 添加阻止规则
    BlockRemoteAddresses(rule_name, address);

    // 设定定时器自动移除阻止规则
    std::shared_ptr<boost::asio::deadline_timer> unblock_timer_ptr;
    unblock_timer_ptr = std::make_shared<boost::asio::deadline_timer>(io_context);
    unblock_timer_ptr->expires_from_now(boost::posix_time::seconds(block_time));
    unblock_timer_ptr->async_wait(
        boost::bind(&UnblockRemoteAddresses, unblock_timer_ptr, rule_name)
    );
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
        g_logger->info("Invalid address : []", address);
        return;
    }
    if (IsWhitelistAddress(address) == true)
    {
        g_logger->info("Whitelist address : []", address);
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
            PlanBlockRemoteAddresses(io_context, address, program_setting::block_time);
            find_iter->second.set_blocked_interval(program_setting::block_time);
            find_iter->second.set_expire_interval(program_setting::block_time);
        }
    }
    delete_expire_keys(address_count);
}

void SubscribeSystemAuthEvent(boost::asio::io_context* io_context)
{
    std::map<std::string, block_address_status> address_count;
    g_logger->info("Subscribe RDP auth failed event.");
    SubscribeSystemEvent os_event_log;
    // 订阅RDP登录失败事件
    if (os_event_log.SubscribeRDPAuthFailedEvent() != true)
    {
        g_logger->error("SubscribeRDPAuthFailedEvent failed.");
        return;
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
                ProcessRemoteAddresses(*io_context, address_count, address);
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
        // std::cout << err.what() << std::endl;
        g_logger->error("Loading configuration file error.");
        g_logger->error(err.what());
    }
    return bRet;
}

// 确保程序目录为工作目录
void ensure_work_dir()
{
    std::string work_dir = get_self_dir_path();
    set_work_dir(work_dir);
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
            std::exit(1);
        }

        if (var_map.count("config") == 0)
        {
            // 如果未设定加载的配置文件默认为 config.ini
            program_setting::config_file_path = "config.ini";
        }
        if (load_config_file(program_setting::config_file_path) == false)
        {
            std::exit(1);
        }
    }
    catch (std::exception& err)
    {
        // std::cout << err.what() << std::endl;
        g_logger->error("Parsing command line error.");
        g_logger->error(err.what());
        std::exit(1);
    }
}

int main(int argc, char* argv[])
{
    // 确保程序目录为工作目录
    ensure_work_dir();

    // 初始化logger
    init_logger();
    g_logger->info("RDPBlocker Version {}", program_setting::program_version);

    // 解析命令行参数
    prase_argv(argc, argv);

    // 确保系统中只有一个RDPBlocker运行，以免互相干扰。
    HANDLE hAppMutex = NULL;
    if (LockAppMutex(hAppMutex, "RDPBlocker-locker") == false)
    {
        g_logger->warn("RDPBlocker already running in the system");
        g_logger->warn("Please do not start more than one process at the same time");
        return 1;
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
    UnLockAppMutex(hAppMutex);
    return 0;
}
