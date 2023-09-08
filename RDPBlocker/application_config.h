#ifndef __APPLICATION_CONFIG__
#define __APPLICATION_CONFIG__

#include <map>
#include <set>
#include <string>
#include <vector>

#include <boost/nowide/fstream.hpp>

#include "boost_regex_utils.h"
#include "logger.h"
#include "random_utils.h"
#include "version.h"
#include "yaml.h"

class ApplicationBlockConfig
{
    public:
    // 阻挡阈值
    int m_threshold;
    // 阻挡时间
    int m_block_time;
    // 记录过期时间
    int m_expire_time;
    // 随机延迟
    int m_random_delay_min;
    int m_random_delay_max;

    ApplicationBlockConfig();
    ~ApplicationBlockConfig();

    int block_time() const;
    int expire_time() const;
};

class ApplicationWorkstationNameConfig
{
    public:
    // 启用检查主机名功能
    bool m_enable_check;
    // 检查绑定列表
    bool m_check_bind;
    // 自动绑定
    bool m_auto_bind;
    // 用户绑定主机名
    std::map<std::string, std::string> m_bind_table;
    // 主机名阻挡名单
    std::set<std::string> m_blocklist;
    // 主机名白名单
    std::set<std::string> m_whitelist;

    ApplicationWorkstationNameConfig();
    ~ApplicationWorkstationNameConfig();

    // 检查是否在阻止名单中
    bool is_block_name(const std::string& value) const;
    // 检查是否在白名单中
    bool is_white_name(const std::string& value) const;

    // 创建绑定记录
    void create_bind_record(const std::string& username,
                            const std::string& workstation_name);
    // 检查是否存在绑定记录
    bool exists_bind_record(const std::string& username) const;

    // 检查与绑定记录是否匹配
    bool check_bind_record(const std::string& username,
                           const std::string& workstation_name) const;
};

class ApplicationIPConfig
{
    public:
    // IP白名单列表
    std::vector<boost::regex> m_whitelist;

    ApplicationIPConfig();
    ~ApplicationIPConfig();

    // 验证IP是否在白名单中
    bool is_white_address(const std::string& ip_address);
};

class ApplicationConfig
{
    public:
    // 程序常量
    const std::string m_mutex_name;
    const std::string m_rule_prefix;
    const std::string m_build_version;
    const std::string m_build_date;

    // 配置文件路径
    std::string m_file_path;

    // 阻挡配置
    ApplicationBlockConfig m_block;

    // 工作站名配置
    ApplicationWorkstationNameConfig m_workstation_name_config;

    // log 配置
    LoggerConfig m_logger_setting;

    // IP 配置
    ApplicationIPConfig m_ip_config;

    ApplicationConfig();
    ~ApplicationConfig();

    bool load_config_file();

    static void read_binary_file(std::vector<char>& buffer,
                                 const std::string& filepath);
};

extern ApplicationConfig g_config;

#endif  //__APPLICATION_CONFIG__
