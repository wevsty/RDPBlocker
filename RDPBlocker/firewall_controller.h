#ifndef __WINDOWS_FIREWALL_CONTROLLER__
#define __WINDOWS_FIREWALL_CONTROLLER__

#include <string>

#include <atlcomcli.h>
#include <netfw.h>
#include <windows.h>

#include "utf_convert.h"
#include "boost_regex_utils.h"
#include "logger.h"

namespace firewall_controller
{
std::shared_ptr<WCHAR> ConvertStdStringToSystemString(const std::string& src);
std::string ConvertSystemStringToStdString(const BSTR ptr);

CComPtr<INetFwPolicy2> GetFireWallPolicy();
CComPtr<INetFwRules> GetFireWallRules(CComPtr<INetFwPolicy2>& pPolicy);
CComPtr<INetFwRule> CreateFireWallRule();
bool SetFireWallRuleName(CComPtr<INetFwRule>& pRule, const std::string& name);
bool SetFireWallRuleDescription(
    CComPtr<INetFwRule>& pRule,
    const std::string& description = "Auto Create Rule.");
bool SetFireWallRuleDirection(
    CComPtr<INetFwRule>& pRule,
    const NET_FW_RULE_DIRECTION flag = NET_FW_RULE_DIR_IN);
bool SetFireWallRuleProtocol(CComPtr<INetFwRule>& pRule,
                             const LONG flag = NET_FW_IP_PROTOCOL_TCP);
bool SetFireWallRuleLocalPorts(CComPtr<INetFwRule>& pRule,
                               const std::string& port);
bool SetFireWallRuleRemoteAddresses(CComPtr<INetFwRule>& pRule,
                                    const std::string& address);
bool SetFireWallRuleAction(CComPtr<INetFwRule>& pRule,
                           const NET_FW_ACTION flag = NET_FW_ACTION_ALLOW);
bool SetFireWallRuleEnabled(CComPtr<INetFwRule>& pRule,
                            const bool enable = false);
bool AddRule(CComPtr<INetFwRules>& rules, CComPtr<INetFwRule>& pRule);
bool DeleteRule(CComPtr<INetFwRules>& rules, const std::string& rule_name);
// 阻挡入站IP
bool BlockInboundIP(const std::string& name, const std::string& address);
// 按照规则名称删除防火墙规则
bool DeleteRuleByName(const std::string& rule_name);
// 按照正则表达式删除防火墙规则
bool DeleteRulesByRegex(const std::string& expr);
// 判断防火墙是否已开启
bool IsFireWallEnabled();
}  // namespace firewall_controller

#endif  //__WINDOWS_FIREWALL_BLOCK__
