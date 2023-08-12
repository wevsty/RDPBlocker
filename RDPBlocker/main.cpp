#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
// Set Windows SDK Version
#include <SDKDDKVer.h>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/program_options.hpp>
//#include <boost/property_tree/ini_parser.hpp>
//#include <boost/property_tree/ptree.hpp>
#include <boost/thread.hpp>

#include "yaml.h"
#include "application_mutex.h"
#include "block_address_status.h"
#include "boost_xml_utils.h"
#include "expires_map_utils.h"
#include "firewall_controller.h"
#include "logger.h"
#include "PECheckSum.h"
#include "program_path_utils.h"
#include "self_exit_code.h"
#include "random_utils.h"
#include "system_com_utils.h"
#include "subscribe_system_event.h"


namespace program_setting {
    // 固定常量
    const std::string app_mutex_name = "RDPBlocker_mutex";
    const std::string program_version = "1.2.0.0";
    const std::string rule_prefix = "AUTO_BLOCKED_";

    // 配置文件路径
    std::string config_file_path;

    // log 配置
    logger_options logger_setting;

    // 阻挡阈值
    int block_threshold;
    // 阻挡时间
    int block_time;
    // 检查主机名
    bool check_workstation;
    // IP白名单列表
    std::vector<boost::regex> ip_whitelist;
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
    for (const auto& expr : program_setting::ip_whitelist)
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

void ProcessRDPAuthFailedEvent(boost::asio::io_context* io_context_ptr)
{
    boost::asio::io_context& io_context = *io_context_ptr;
    std::map<std::string, block_address_status> address_count;
    // g_logger->info("Subscribe RDP auth failed event.");
    g_logger->info("Subscribe RDPAuthFailedEvent.");
    RDPAuthFailedEvent auth_failed_evt;
    // 订阅RDP登录失败事件
    if (auth_failed_evt.Subscribe() != true)
    {
        g_logger->error("Subscribe RDPAuthFailedEvent failed.");
        std::exit(EXIT_CODE::SUBSCRIBE_EVENT_ERROR);
    }
    while (true)
    {
        g_logger->debug("Wait event.");
        std::string event_xml_data;
        auth_failed_evt.Pop(event_xml_data);

        std::map<std::string, std::string> event_attr;
        EventDataToMap(event_xml_data, event_attr);
        std::string address = event_attr["IpAddress"];
        if (address != "-")
        {
            g_logger->info("Address auth failed : {}", address);
            ProcessRemoteAddresses(io_context, address_count, address);
        }
    }
}

void ProcessRDPAuthSucceedEvent(boost::asio::io_context* io_context_ptr)
{
    boost::asio::io_context& io_context = *io_context_ptr;
    std::map<std::string, block_address_status> address_count;
    std::map<std::string, std::string> workstation_name_map;
    g_logger->info("Subscribe RDPAuthSucceedEvent.");
    RDPAuthSucceedEvent auth_succeed_evt;
    // 订阅RDP登录失败事件
    if (auth_succeed_evt.Subscribe() != true)
    {
        g_logger->error("Subscribe RDPAuthSucceedEvent failed.");
        std::exit(EXIT_CODE::SUBSCRIBE_EVENT_ERROR);
    }
    while (true)
    {
        g_logger->debug("Wait event.");
        std::string event_xml_data;
        auth_succeed_evt.Pop(event_xml_data);

        std::map<std::string, std::string> event_attr;
        EventDataToMap(event_xml_data, event_attr);
        std::string workstation_name = event_attr["WorkstationName"];
        std::string user_name = event_attr["TargetUserName"];
        std::string address = event_attr["IpAddress"];
        if (workstation_name != "" && user_name != "" && address != "-")
        {
            auto it = workstation_name_map.find(user_name);
            if (it == workstation_name_map.end())
            {
                workstation_name_map[user_name] = workstation_name;
            }
            else
            {
                if (it->second == workstation_name)
                {
                    continue;
                }
                else
                {
                    g_logger->info("Check workstation name failed : {}", workstation_name);
                    //g_logger->info("Block address : {}", address);
                    ProcessRemoteAddresses(io_context, address_count, address);
                }
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
        YAML::Node config = YAML::LoadFile(file_path);

        // 日志配置
        const YAML::Node& node_logs = config["log"];
        program_setting::logger_setting.filename = node_logs["filename"].as<std::string>();
        program_setting::logger_setting.max_size = node_logs["max_size"].as<int>();
        program_setting::logger_setting.max_files = node_logs["max_files"].as<int>();
        std::string output_level = node_logs["level"].as<std::string>();
        program_setting::logger_setting.set_level_string(output_level);

        // 阻挡配置
        program_setting::block_threshold = config["block_threshold"].as<int>();
        program_setting::block_time = config["block_time"].as<int>();
        program_setting::check_workstation = config["check_workstation"].as<bool>();

        // 白名单配置
        const YAML::Node& node_ip_whiltelist = config["IP_whitelist"];
        for (unsigned i = 0; i < node_ip_whiltelist.size(); i++) {
            std::string regex_string = node_ip_whiltelist[i].as<std::string>();
            boost::regex regex_obj(regex_string);
            program_setting::ip_whitelist.push_back(regex_obj);
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
        std::cout << "Check file failed" << std::endl;
        std::cout << "The file has been corrupted" << std::endl;
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
            // 如果未设定加载的配置文件默认为 config.yaml
            program_setting::config_file_path = "config.yaml";
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
    init_global_logger(program_setting::logger_setting);
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

    // 检测防火墙是否已开启
    if (firewall_controller::IsFireWallEnabled() == false)
    {
        g_logger->error("Windows Firewall must be turn on");
        return EXIT_CODE::FIREWALL_ERROR;
    }

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
        boost::bind(&ProcessRDPAuthFailedEvent, &io_context)
    );
    if (program_setting::check_workstation)
    {
        boost::thread event_thread(
            boost::bind(&ProcessRDPAuthSucceedEvent, &io_context)
        );
    }
    
    event_thread.join();
    io_thread.join();
    return EXIT_CODE::SUCCESS;
}
