#include "application_config.h"

// 全局变量
ApplicationConfig g_config = ApplicationConfig();

ApplicationBlockConfig::ApplicationBlockConfig()
    : m_block_time(600), m_random_delay_min(0), m_random_delay_max(30)
{
}

ApplicationBlockConfig::~ApplicationBlockConfig()
{
}

int ApplicationBlockConfig::block_time() const
{
    int random_delay = random_int(m_random_delay_min, m_random_delay_max);
    return m_block_time + random_delay;
}

ApplicationFailBanConfig::ApplicationFailBanConfig()
    : m_enable(true), m_threshold(3), m_expire_time(900)
{
}

ApplicationFailBanConfig::~ApplicationFailBanConfig()
{
}

int ApplicationFailBanConfig::expire_time() const
{
    return m_expire_time;
}

ApplicationWorkstationNameConfig::ApplicationWorkstationNameConfig()
    : m_enable_check(false),
      m_check_bind(true),
      m_auto_bind(true),
      m_bind_table(),
      m_blocklist(),
      m_whitelist()
{
}

ApplicationWorkstationNameConfig::~ApplicationWorkstationNameConfig()
{
}

bool ApplicationWorkstationNameConfig::is_block_name(
    const std::string& value) const
{
    auto it = m_blocklist.find(value);
    if (it != m_blocklist.end())
    {
        // 说明在阻挡名单内
        return true;
    }
    return false;
}

bool ApplicationWorkstationNameConfig::is_white_name(
    const std::string& value) const
{
    auto it = m_whitelist.find(value);
    if (it != m_whitelist.end())
    {
        // 说明在阻挡名单内
        return true;
    }
    return false;
}

void ApplicationWorkstationNameConfig::create_bind_record(
    const std::string& username,
    const std::string& workstation_name)
{
    m_bind_table[username] = workstation_name;
}

bool ApplicationWorkstationNameConfig::exists_bind_record(
    const std::string& username) const
{
    auto it = m_bind_table.find(username);
    if (it != m_bind_table.end())
    {
        // 说明存在记录
        return true;
    }
    return false;
}

bool ApplicationWorkstationNameConfig::check_bind_record(
    const std::string& username,
    const std::string& workstation_name) const
{
    auto it = m_bind_table.find(username);
    if (it != m_bind_table.end())
    {
        // 说明存在记录
        const std::string& regexp = it->second;
        if (regex_is_match(regexp, workstation_name))
        {
            return true;
        }
    }
    return false;
}
ApplicationLoggerConfig::ApplicationLoggerConfig()
    : m_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v"), m_level(spdlog::level::debug)
{
}

ApplicationLoggerConfig::~ApplicationLoggerConfig()
{
}

void ApplicationLoggerConfig::set_level(const std::string& level_string)
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

std::string ApplicationLoggerConfig::get_level() const
{
    return std::string(spdlog::level::to_short_c_str(m_level));
}

void ApplicationLoggerConfig::apply(std::shared_ptr<spdlog::logger>& logger)
{
    // logger->set_pattern(m_pattern);
    logger->set_level(m_level);
}

ApplicationIPConfig::ApplicationIPConfig() : m_whitelist()
{
}

ApplicationIPConfig::~ApplicationIPConfig()
{
}

bool ApplicationIPConfig::is_white_address(const std::string& ip_address) const
{
    for (const auto& expression : m_whitelist)
    {
        if (regex_is_match(expression, ip_address) == true)
        {
            return true;
        }
    }
    return false;
}

ApplicationConfig::ApplicationConfig()
    : m_mutex_name("RDPBlocker_mutex"),
      m_rule_prefix("AUTO_BLOCKED_"),
      m_build_version(APPLICATION_FILE_VERSION_STRING),
      m_build_date(__DATE__ " " __TIME__),
      m_file_path("config.yaml"),
      m_block(),
      m_fail_ban(),
      m_workstation_name_config(),
      m_logger_config(),
      m_ip_config()
{
}

ApplicationConfig::~ApplicationConfig()
{
}

bool ApplicationConfig::load_config_file()
{
    bool bret = false;

    // 读取文件
    std::string file_text;
    std::vector<char> file_buffer;
    bool b_result = read_binary_file(file_buffer, m_file_path);
    if (b_result == false)
    {
        std::cout << "Read file failed." << std::endl;
        return bret;
    }
    file_text.assign(file_buffer.data(), file_buffer.size());

    // 解析配置文件
    try
    {
        YAML::Node root_node = YAML::Load(file_text);

        // 阻挡配置
        const YAML::Node& node_block = root_node["block"];
        m_block.m_block_time = node_block["block_time"].as<int>();
        m_block.m_random_delay_min = node_block["random_delay_min"].as<int>();
        m_block.m_random_delay_max = node_block["random_delay_max"].as<int>();

        // 失败阻挡配置
        const YAML::Node& node_failban = root_node["failban"];
        m_fail_ban.m_enable = node_failban["enable"].as<bool>();
        m_fail_ban.m_threshold = node_failban["threshold"].as<int>();
        m_fail_ban.m_expire_time = node_failban["expire_time"].as<int>();

        // 主机名配置
        const YAML::Node& node_workstation_name = root_node["workstation_name"];
        m_workstation_name_config.m_enable_check =
            node_workstation_name["enable_check"].as<bool>();
        m_workstation_name_config.m_check_bind =
            node_workstation_name["check_bind"].as<bool>();
        m_workstation_name_config.m_auto_bind =
            node_workstation_name["auto_bind"].as<bool>();

        // 载入绑定列表
        const YAML::Node& node_user_bind = node_workstation_name["user_bind"];
        for (YAML::const_iterator it = node_user_bind.begin();
             it != node_user_bind.end(); ++it)
        {
            std::string user_name = it->first.as<std::string>();
            std::string bind_name = it->second.as<std::string>();
            m_workstation_name_config.m_bind_table[user_name] = bind_name;
        }
        // 主机名阻挡名单
        const YAML::Node& node_workstation_name_blocklist =
            node_workstation_name["blocklist"];
        for (unsigned int i = 0; i < node_workstation_name_blocklist.size();
             i++)
        {
            std::string workstation_name =
                node_workstation_name_blocklist[i].as<std::string>();
            m_workstation_name_config.m_blocklist.insert(workstation_name);
        }
        // 主机名白名单
        const YAML::Node& node_workstation_name_whitelist =
            node_workstation_name["whitelist"];
        for (unsigned int i = 0; i < node_workstation_name_whitelist.size();
             i++)
        {
            std::string workstation_name =
                node_workstation_name_whitelist[i].as<std::string>();
            m_workstation_name_config.m_whitelist.insert(workstation_name);
        }

        // 日志配置
        const YAML::Node& node_logs = root_node["log"];
        std::string level_string = node_logs["level"].as<std::string>();
        m_logger_config.set_level(level_string);

        // IP配置
        const YAML::Node& node_ip_address = root_node["IP_Address"];
        const YAML::Node& node_ip_whiltelist = node_ip_address["whitelist"];
        for (unsigned int i = 0; i < node_ip_whiltelist.size(); i++)
        {
            std::string expression = node_ip_whiltelist[i].as<std::string>();
            boost::regex expression_object(expression);
            m_ip_config.m_whitelist.push_back(expression_object);
        }

        bret = true;
    }
    catch (const YAML::Exception& err)
    {
        std::cout << "Exception when load file." << std::endl;
        std::cout << err.what() << std::endl;
    }

    return bret;
}

bool ApplicationConfig::read_binary_file(std::vector<char>& buffer,
                                         const std::string& filepath)
{
    bool bret = false;
    boost::nowide::fstream fs(filepath, std::ios::in | std::ios::binary);
    if (fs.is_open())
    {
        fs.seekg(0, std::ios::end);
        std::size_t n_filesize = fs.tellg();
        if (n_filesize > 0)
        {
            fs.seekg(0, std::ios::beg);
            buffer.resize(n_filesize);
            fs.read(buffer.data(), n_filesize);
        }
        fs.close();
        bret = true;
    }
    return bret;
}
