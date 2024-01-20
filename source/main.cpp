#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
// Set Windows SDK Version
#include <SDKDDKVer.h>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/nowide/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <boost/version.hpp>

#include "application_config.h"
#include "application_exit_code.h"
#include "application_mutex.h"
#include "boost_xml_utils.h"
#include "firewall_controller.h"
#include "logger.h"
#include "pe_checksum.h"
#include "program_path_utils.h"
#include "random_utils.h"
#include "remote_address_status.h"
#include "subscribe_system_event.h"
#include "system_com_utils.h"
#include "utf_convert.h"
#include "yaml.h"

class BlockRemoteAddressTask
    : public std::enable_shared_from_this<BlockRemoteAddressTask>
{
    private:
    boost::asio::io_context& m_io_context;
    boost::asio::steady_timer m_wait_timer;

    public:
    BlockRemoteAddressTask(boost::asio::io_context& io_context)
        : m_io_context(io_context), m_wait_timer(io_context)
    {
        m_wait_timer.expires_at(boost::asio::steady_timer::time_point::max());
    }

    ~BlockRemoteAddressTask()
    {
    }

    void block(const std::string& rule_name, const std::string& block_address)
    {
        g_logger->info("Add rule {}", rule_name);
        firewall_controller::BlockInboundIP(rule_name, block_address);
    }

    void unblock(const std::string& rule_name)
    {
        g_logger->info("Delete rule {}", rule_name);
        firewall_controller::DeleteRuleByName(rule_name);
    }

    void async_block(const std::string& rule_name,
                     const std::string& block_address)
    {
        m_io_context.post(boost::bind(&BlockRemoteAddressTask::block,
                                      shared_from_this(), rule_name,
                                      block_address));
    }

    void async_unblock(const std::string& rule_name, const int delay_time)
    {
        m_wait_timer.expires_from_now(std::chrono::seconds(delay_time));
        m_wait_timer.async_wait(boost::bind(&BlockRemoteAddressTask::unblock,
                                            shared_from_this(), rule_name));
    }
};

void create_block_remote_address_tasks(boost::asio::io_context& io_context,
                                       const std::string& remote_address,
                                       const int block_time)
{
    // 常量
    constexpr unsigned int RULE_RANDOM_CHARS_LENGTH = 8;

    // 生成随机的规则名
    std::string rule_name = g_config.m_rule_prefix;
    rule_name += random_string(RULE_RANDOM_CHARS_LENGTH);

    std::shared_ptr<BlockRemoteAddressTask> block_task =
        std::make_shared<BlockRemoteAddressTask>(io_context);

    if (block_time > 0)
    {
        // 添加阻止规则
        block_task->async_block(rule_name, remote_address);

        // 超时自动删除规则
        block_task->async_unblock(rule_name, block_time);
    }
    else
    {
        g_logger->warn("block_time <= 0");
    }
}

// 验证IP合法性
bool is_vaild_ip_address(const std::string& value)
{
    try
    {
        boost::asio::ip::address addr =
            boost::asio::ip::address::from_string(value);
        return true;
    }
    catch (const boost::system::system_error& error)
    {
        std::cout << error.what() << std::endl;
        return false;
    }
}

void block_remote_addresses_when_login_failed(
    boost::asio::io_context& io_context,
    std::map<std::string, RemoteAddressStatus>& remote_address_map,
    const std::string& remote_address)
{
    // 校验IP地址
    if (is_vaild_ip_address(remote_address) == false)
    {
        g_logger->info("Invalid address : {}", remote_address);
        return;
    }
    // 检测白名单
    if (g_config.m_ip_config.is_white_address(remote_address) == true)
    {
        g_logger->info("Whitelist address : {}", remote_address);
        return;
    }
    // 寻找对应的地址记录
    auto find_iter = remote_address_map.find(remote_address);
    if (find_iter == remote_address_map.end())
    {
        const int n_expire_time = g_config.m_fail_ban.expire_time();
        RemoteAddressStatus st;
        st.set_expire_time(n_expire_time);
        remote_address_map[remote_address] = st;
        find_iter = remote_address_map.find(remote_address);
    }
    // 延长记录保存时间
    find_iter->second.reset_expire_timer();
    // 记录次数增加1
    int temp_count = find_iter->second.get_count() + 1;
    find_iter->second.set_count(temp_count);
    // 判断是否达到阻挡阈值
    if (find_iter->second.get_count() >= g_config.m_fail_ban.m_threshold)
    {
        if (find_iter->second.is_blocked() == false)
        {
            g_logger->info("Block address : {}", remote_address);
            const int n_block_time = g_config.m_block.block_time();
            create_block_remote_address_tasks(io_context, remote_address,
                                              n_block_time);
            find_iter->second.set_block_time(n_block_time);
        }
        else
        {
            g_logger->debug("Blocked address : {}", remote_address);
        }
    }
}

void block_remote_addresses_when_login_succeed(
    boost::asio::io_context& io_context,
    const std::string& remote_address)
{
    // 校验IP地址
    if (is_vaild_ip_address(remote_address) == false)
    {
        g_logger->info("Invalid address : {}", remote_address);
        return;
    }
    // 检测白名单
    if (g_config.m_ip_config.is_white_address(remote_address) == true)
    {
        g_logger->info("Whitelist address : {}", remote_address);
        return;
    }

    // 阻止地址
    g_logger->info("Block address : {}", remote_address);
    const int n_block_time = g_config.m_block.block_time();
    create_block_remote_address_tasks(io_context, remote_address, n_block_time);
}

void delete_expire_remote_address(
    std::map<std::string, RemoteAddressStatus>& map)
{
    for (auto it = map.begin(); it != map.end();)
    {
        if (it->second.is_blocked() == true)
        {
            it++;
            continue;
        }
        if (it->second.is_expired() == true)
        {
            g_logger->debug("Delete expire : {}", it->first);
            // delete item
            it = map.erase(it);
        }
        else
        {
            it++;
        }
    }
}

void process_rdp_auth_failed_event(boost::asio::io_context* io_context_ptr)
{
    boost::asio::io_context& io_context = *io_context_ptr;
    std::map<std::string, RemoteAddressStatus> remote_address_map;
    g_logger->info("Subscribe RDPAuthFailedEvent.");
    RDPAuthFailedEvent auth_failed_evt;
    // 订阅RDP登录失败事件
    if (auth_failed_evt.Subscribe() != true)
    {
        g_logger->error("Subscribe RDPAuthFailedEvent failed.");
        std::exit(APPLICATION_EXIT_CODE::FAILED);
    }
    while (true)
    {
        g_logger->debug("Wait RDPAuthFailedEvent.");
        std::string event_xml_data;
        auth_failed_evt.Pop(event_xml_data);

        // 删除过期的记录
        delete_expire_remote_address(remote_address_map);

        std::map<std::string, std::string> event_attr;
        EventDataToMap(event_xml_data, event_attr);
        std::string remote_ip_address = event_attr["IpAddress"];

        // 如果IP为空则直接跳过
        if (remote_ip_address == "-" || remote_ip_address.empty() == true)
        {
            continue;
        }
        g_logger->info("Auth failed : {}", remote_ip_address);
        block_remote_addresses_when_login_failed(io_context, remote_address_map,
                                                 remote_ip_address);
    }
}

std::string get_local_workstation_name()
{
    DWORD dwSize = dwSize = MAX_COMPUTERNAME_LENGTH + 1;
    std::shared_ptr<WCHAR[]> buffer = std::make_shared<WCHAR[]>(dwSize);
    ZeroMemory(buffer.get(), dwSize * sizeof(WCHAR));
    GetComputerNameW(buffer.get(), &dwSize);
    std::string hostname = boost::locale::conv::utf_to_utf<char>(buffer.get());
    return hostname;
}

void process_rdp_auth_succeed_event(boost::asio::io_context* io_context_ptr)
{
    boost::asio::io_context& io_context = *io_context_ptr;
    std::string local_hostname = get_local_workstation_name();
    g_logger->info("Subscribe RDPAuthSucceedEvent.");
    RDPAuthSucceedEvent auth_succeed_evt;
    // 订阅RDP登录成功事件
    if (auth_succeed_evt.Subscribe() != true)
    {
        g_logger->error("Subscribe RDPAuthSucceedEvent failed.");
        std::exit(APPLICATION_EXIT_CODE::FAILED);
    }
    while (true)
    {
        g_logger->debug("Wait RDPAuthSucceedEvent.");
        std::string event_xml_data;
        auth_succeed_evt.Pop(event_xml_data);

        // 收到事件
        std::map<std::string, std::string> event_attr;
        EventDataToMap(event_xml_data, event_attr);
        std::string logon_type = event_attr["LogonType"];
        std::string workstation_name = event_attr["WorkstationName"];
        std::string user_name = event_attr["TargetUserName"];
        std::string remote_ip_address = event_attr["IpAddress"];
        // logon_type 3 is network logon
        // https://learn.microsoft.com/en-us/windows/security/threat-protection/auditing/event-4624
        if (logon_type != "3")
        {
            continue;
        }
        // 如果IP为空则直接跳过
        if (remote_ip_address == "-" || remote_ip_address.empty() == true)
        {
            continue;
        }

        g_logger->info("Check login {} : {}", user_name, workstation_name);

        // 检查工作站名白名单
        if (g_config.m_workstation_name_config.is_white_name(workstation_name))
        {
            // 说明在白名单内
            g_logger->info("Whitelist login {} : {}", user_name,
                           workstation_name);
            continue;
        }
        // 检查工作站名阻挡名单
        if (g_config.m_workstation_name_config.is_block_name(workstation_name))
        {
            // 说明在阻挡名单内
            g_logger->info("Blocklist login {} : {}", user_name,
                           workstation_name);
            block_remote_addresses_when_login_succeed(io_context,
                                                      remote_ip_address);
            continue;
        }
        // 进行绑定检查
        if (g_config.m_workstation_name_config.m_check_bind == false)
        {
            // 如果不进行绑定检查则直接跳过
            // 直接允许登录
            g_logger->info("Allow login {} : {}", user_name, workstation_name);
            continue;
        }

        if (g_config.m_workstation_name_config.exists_bind_record(user_name) ==
            false)
        {
            // 不存在绑定记录
            if (g_config.m_workstation_name_config.m_auto_bind == true)
            {
                // 创建绑定关系
                g_config.m_workstation_name_config.create_bind_record(
                    user_name, workstation_name);
                g_logger->info("Auto bind {} : {}", user_name,
                               workstation_name);
            }
        }

        if (g_config.m_workstation_name_config.check_bind_record(
                user_name, workstation_name) == true)
        {
            // 允许登录
            g_logger->info("Allow login {} : {}", user_name, workstation_name);
            continue;
        }
        else
        {
            // 不允许登录
            g_logger->info("Block login {} : {}", user_name, workstation_name);
            block_remote_addresses_when_login_succeed(io_context,
                                                      remote_ip_address);
        }
    }  // while loop
}

// 确保程序目录为工作目录
void ensure_work_dir()
{
    std::string work_dir = self_directory_path();
    set_work_dir(work_dir);
}

// 对程序自身进行校验
void check_program_file()
{
    std::string self_path = self_file_path();
    std::wstring ws_path = utf_to_utf<wchar_t>(self_path);
    const bool status = check_pe_checksum(ws_path);
    if (status == false)
    {
        std::cout << "Check program file failed" << std::endl;
        std::cout << "The file has been corrupted" << std::endl;
        std::exit(APPLICATION_EXIT_CODE::FAILED);
    }
}

void display_build_info()
{
    std::cout << "RDPBlocker build info:" << std::endl;
    std::cout << "application version: " << g_config.m_build_version
              << std::endl;
    std::cout << "build date: " << g_config.m_build_date << std::endl;
#if defined(_MSC_VER)
    std::cout << "build compiler: "
              << "MSVC " << _MSC_VER << std::endl;
#endif
    std::cout << std::endl << "Library version:" << std::endl;
    std::cout << "boost: " << BOOST_VERSION / 100000 << '.'
              << BOOST_VERSION / 100 % 1000 << '.' << BOOST_VERSION % 100
              << std::endl;
    std::cout << "spdlog: " << SPDLOG_VER_MAJOR << '.' << SPDLOG_VER_MINOR
              << '.' << SPDLOG_VER_PATCH << std::endl;
}

// 解析命令行参数
void prase_argv(int argc, char* argv[])
{
    try
    {
        // namespace BPO = boost::program_options;
        boost::program_options::options_description options(
            "RDPBlocker command options");
        options.add_options()("help", "Produce help message");
        options.add_options()("version", "Show version info");
        options.add_options()(
            "config",
            boost::program_options::value<std::string>(&g_config.m_file_path),
            "config file path");
        // parse program options
        boost::program_options::variables_map var_map;
        boost::program_options::store(
            boost::program_options::parse_command_line(argc, argv, options),
            var_map);
        boost::program_options::notify(var_map);

        if (var_map.count("help") != 0)
        {
            std::cout << options << std::endl;
            std::exit(APPLICATION_EXIT_CODE::SUCCESS);
        }
        if (var_map.count("version") != 0)
        {
            display_build_info();
            std::exit(APPLICATION_EXIT_CODE::SUCCESS);
        }
        if (var_map.count("config") == 0)
        {
            // 如果未设定加载的配置文件默认为 config.yaml
            g_config.m_file_path = "config.yaml";
        }
        if (g_config.load_config_file() == false)
        {
            std::exit(APPLICATION_EXIT_CODE::FAILED);
        }
        // 应用 logger 设置
        g_config.m_logger_config.apply(g_logger);
    }
    catch (const boost::program_options::error& err)
    {
        std::cout << "Parsing command line error" << std::endl;
        std::cout << err.what() << std::endl;
        std::exit(APPLICATION_EXIT_CODE::FAILED);
    }
}

int main(int argc, char* argv[])
{
    // 使filesystem支持UTF-8
    boost::nowide::nowide_filesystem();
    // 确保程序目录为工作目录
    ensure_work_dir();

    // 对程序文件进行自校验
    check_program_file();

    // 初始化logger
    initialize_global_logger(g_logger);

    // 解析命令行参数
    prase_argv(argc, argv);

    // 确保系统中只有一个RDPBlocker运行，以免互相干扰。
    ApplicationMutex app_mutex;
    if (app_mutex.lock(g_config.m_mutex_name) == false)
    {
        g_logger->warn("RDPBlocker already running in the system");
        return APPLICATION_EXIT_CODE::FAILED;
    }

    // 初始化COM
    g_logger->debug("Initialization COM");
    SystemComInitialize sys_com_init;

    // 检测防火墙是否已开启
    if (firewall_controller::IsFireWallEnabled() == false)
    {
        g_logger->error("Windows Firewall must be turn on");
        return APPLICATION_EXIT_CODE::FAILED;
    }

    // 启动流程
    g_logger->info("RDPBlocker version {}", g_config.m_build_version);

    // 重启时自动删除遗留规则
    g_logger->debug("Remove existing auto-blocking rules");
    firewall_controller::DeleteRulesByRegex(g_config.m_rule_prefix);
    g_logger->debug("Remove auto-blocking rules finish");

    boost::asio::io_context io_context;

    // 创建一个空任务 防止io_context.run()立刻返回
    boost::asio::io_context::work idle_work(io_context);

    // 启动IO线程
    boost::thread io_thread(
        boost::bind(&boost::asio::io_context::run, &io_context));

    // 启动事件订阅线程

    // 检查配置是否启用工作站名检查功能
    if (g_config.m_fail_ban.m_enable)
    {
        // 启动登陆失败事件处理线程
        boost::thread thread_process_auth_failed_event(
            boost::bind(&process_rdp_auth_failed_event, &io_context));
    }

    // 检查配置是否启用工作站名检查功能
    if (g_config.m_workstation_name_config.m_enable_check)
    {
        // 启动登陆成功事件处理线程
        boost::thread thread_process_auth_succeed_event(
            boost::bind(&process_rdp_auth_succeed_event, &io_context));
    }

    io_thread.join();
    return APPLICATION_EXIT_CODE::SUCCESS;
}
