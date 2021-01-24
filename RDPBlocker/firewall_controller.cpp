#include "firewall_controller.h"

namespace firewall_controller {
    std::shared_ptr<WCHAR> ConvertStdStringToSystemString(const std::string& src)
    {
        std::wstring ws_src = boost::locale::conv::to_utf<wchar_t>(src, "UTF-8");
        const OLECHAR* bstr_ptr = static_cast<const OLECHAR*>(ws_src.c_str());
        std::shared_ptr<WCHAR> ptr(
            static_cast<BSTR>(SysAllocString(bstr_ptr)),
            SysFreeString
        );
        return ptr;
    }

    std::string ConvertSystemStringToStdString(const BSTR ptr)
    {
        std::wstring ws_src(ptr);
        std::string src = boost::locale::conv::utf_to_utf<char>(ws_src);
        return src;
    }

    CComPtr<INetFwPolicy2> GetFireWallPolicy()
    {
        CComPtr<INetFwPolicy2> pNetFwPolicy2 = NULL;
        HRESULT hr = CoCreateInstance(
            __uuidof(NetFwPolicy2),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(INetFwPolicy2),
            (void**)&pNetFwPolicy2
        );
        if (FAILED(hr))
        {
            g_logger->error("CoCreateInstance failed 0x{:x}.", hr);
            return NULL;
        }
        return pNetFwPolicy2;
    }

    CComPtr<INetFwRules> GetFireWallRules(CComPtr<INetFwPolicy2>& pPolicy)
    {
        HRESULT hr;
        CComPtr<INetFwRules> pFwRules = NULL;
        hr = pPolicy->get_Rules(&pFwRules);
        if (FAILED(hr))
        {
            g_logger->error("INetFwRules::get_Rules failed 0x{:x}.", hr);
            return NULL;
        }
        return pFwRules;
    }

    CComPtr<INetFwRule> CreateFireWallRule()
    {
        HRESULT hr;
        INetFwRule* pFwRule = NULL;
        hr = CoCreateInstance(
            __uuidof(NetFwRule),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(INetFwRule),
            (void**)&pFwRule);
        if (FAILED(hr))
        {
            g_logger->error("CoCreateInstance INetFwRule failed 0x{:x}.", hr);
            return CComPtr<INetFwRule>(NULL);
        }
        return CComPtr<INetFwRule>(pFwRule);
    }


    bool SetFireWallRuleName(CComPtr<INetFwRule>& pRule, const std::string& name)
    {
        std::shared_ptr<WCHAR> bstrRuleName = ConvertStdStringToSystemString(name);
        // Populate the Firewall Rule Name
        HRESULT hr = pRule->put_Name(bstrRuleName.get());
        if (FAILED(hr))
        {
            g_logger->error("INetFwRule::put_Name failed 0x{:x}.", hr);
            return false;
        }
        return true;
    }

    bool SetFireWallRuleDescription(CComPtr<INetFwRule>& pRule, const std::string& description)
    {
        std::shared_ptr<WCHAR> bstrRuleDescription = ConvertStdStringToSystemString(description);
        // Populate the Firewall Rule Description
        HRESULT hr = pRule->put_Description(bstrRuleDescription.get());
        if (FAILED(hr))
        {
            g_logger->error("INetFwRule::put_Description failed 0x{:x}.", hr);
            return false;
        }
        return true;
    }

    bool SetFireWallRuleDirection(CComPtr<INetFwRule>& pRule, const NET_FW_RULE_DIRECTION flag)
    {
        HRESULT hr = pRule->put_Direction(flag);
        if (FAILED(hr))
        {
            g_logger->error("INetFwRule::put_Direction failed 0x{:x}.", hr);
            return false;
        }
        return true;
    }

    bool SetFireWallRuleProtocol(CComPtr<INetFwRule>& pRule, const LONG flag)
    {
        HRESULT hr = pRule->put_Protocol(flag);
        if (FAILED(hr))
        {
            g_logger->error("INetFwRule::put_Protocol failed 0x{:x}.", hr);
            return false;
        }
        return true;
    }

    bool SetFireWallRuleLocalPorts(CComPtr<INetFwRule>& pRule, const std::string& port)
    {
        std::shared_ptr<WCHAR> bstrRulePort = ConvertStdStringToSystemString(port);
        HRESULT hr = pRule->put_LocalPorts(bstrRulePort.get());
        if (FAILED(hr))
        {
            g_logger->error("INetFwRule::put_LocalPorts failed 0x{:x}.", hr);
            return false;
        }
        return true;
    }

    bool SetFireWallRuleRemoteAddresses(CComPtr<INetFwRule>& pRule, const std::string& address)
    {
        std::shared_ptr<WCHAR> bstrRuleAddress = ConvertStdStringToSystemString(address);
        HRESULT hr = pRule->put_RemoteAddresses(bstrRuleAddress.get());
        if (FAILED(hr))
        {
            g_logger->error("INetFwRule::put_RemoteAddresses failed 0x{:x}.", hr);
            return false;
        }
        return true;
    }

    bool SetFireWallRuleAction(CComPtr<INetFwRule>& pRule, const NET_FW_ACTION flag)
    {
        HRESULT hr = pRule->put_Action(flag);
        if (FAILED(hr))
        {
            g_logger->error("INetFwRule::put_Action failed 0x{:x}.", hr);
            return false;
        }
        return true;
    }

    bool SetFireWallRuleEnabled(CComPtr<INetFwRule>& pRule, const bool enable)
    {
        VARIANT_BOOL flag = VARIANT_TRUE;
        if (enable == false)
        {
            flag = VARIANT_FALSE;
        }
        HRESULT hr = pRule->put_Enabled(flag);
        if (FAILED(hr))
        {
            g_logger->error("INetFwRule::put_Enabled failed 0x{:x}.", hr);
            return false;
        }
        return true;
    }

    bool AddRule(CComPtr<INetFwRules>& rules, CComPtr<INetFwRule>& pRule)
    {
        HRESULT hr = rules->Add(pRule);
        if (FAILED(hr))
        {
            g_logger->error("INetFwRules::Add failed 0x{:x}.", hr);
            return false;
        }
        return true;
    }

    bool DeleteRule(CComPtr<INetFwRules>& rules, const std::string& rule_name)
    {
        if (rules == NULL)
        {
            return false;
        }
        else
        {
            std::shared_ptr<WCHAR> bstrRuleName = ConvertStdStringToSystemString(rule_name);
            rules->Remove(bstrRuleName.get());
        }
        return true;
    }

    bool BlockInboundIP(const std::string& name, const std::string& address)
    {
        CComPtr<INetFwPolicy2> pPolicy = GetFireWallPolicy();
        if (pPolicy == NULL)
        {
            return false;
        }
        CComPtr<INetFwRules> pRules = GetFireWallRules(pPolicy);
        if (pRules == NULL)
        {
            return false;
        }
        CComPtr<INetFwRule> pRule = CreateFireWallRule();
        if (SetFireWallRuleName(pRule, name) == false)
        {
            return false;
        }
        if (SetFireWallRuleDescription(pRule, "BlockInboundIP") == false)
        {
            return false;
        }
        if (SetFireWallRuleDirection(pRule, NET_FW_RULE_DIR_IN) == false)
        {
            return false;
        }
        if (SetFireWallRuleProtocol(pRule, NET_FW_IP_PROTOCOL_ANY) == false)
        {
            return false;
        }
        if (SetFireWallRuleRemoteAddresses(pRule, address) == false)
        {
            return false;
        }
        if (SetFireWallRuleAction(pRule, NET_FW_ACTION_BLOCK) == false)
        {
            return false;
        }
        if (SetFireWallRuleEnabled(pRule, true) == false)
        {
            return false;
        }
        if (AddRule(pRules, pRule) == false)
        {
            return false;
        }
        return false;
    }

    bool DeleteRuleByName(const std::string& rule_name)
    {
        CComPtr<INetFwPolicy2> pPolicy = GetFireWallPolicy();
        if (pPolicy == NULL)
        {
            return false;
        }
        CComPtr<INetFwRules> pRules = GetFireWallRules(pPolicy);
        if (pRules == NULL)
        {
            return false;
        }
        std::shared_ptr<WCHAR> bstrRuleName = ConvertStdStringToSystemString(rule_name);
        pRules->Remove(bstrRuleName.get());
        return true;
    }

    bool DeleteRulesByRegex(const std::string& expr)
    {
        boost::regex find_expr(expr);
        HRESULT hr;
        CComPtr<INetFwPolicy2> pPolicy = GetFireWallPolicy();
        if (pPolicy == NULL)
        {
            return false;
        }
        CComPtr<INetFwRules> pRules = GetFireWallRules(pPolicy);
        if (pRules == NULL)
        {
            return false;
        }

        // Obtain the number of Firewall rules
        long RulesCount;
        hr = pRules->get_Count(&RulesCount);
        if (FAILED(hr))
        {
            g_logger->error("INetFwRules::get_Count failed 0x{:x}.", hr);
            return false;
        }

        // Iterate through all of the rules in pRules
        CComPtr<IUnknown> pEnumerator;
        hr = pRules->get__NewEnum(&pEnumerator);
        if (FAILED(hr))
        {
            g_logger->error("INetFwRules::get__NewEnum failed 0x{:x}.", hr);
            return false;
        }

        CComPtr<IEnumVARIANT> pVariant;
        hr = pEnumerator.QueryInterface(&pVariant);
        if (FAILED(hr))
        {
            g_logger->error("get__NewEnum failed to produce IEnumVariant 0x{:x}.", hr);
            return false;
        }

        std::vector<CComBSTR> vt_wait_delete;
        ULONG cFetched = 0;
        for (CComVariant var; pVariant->Next(1, &var, &cFetched) == S_OK; var.Clear())
        {
            CComPtr<INetFwRule> pFwRule;
            if (SUCCEEDED(var.ChangeType(VT_DISPATCH)) &&
                SUCCEEDED(V_DISPATCH(&var)->QueryInterface(IID_PPV_ARGS(&pFwRule))))
            {
                CComBSTR rule_name;
                if (SUCCEEDED(pFwRule->get_Name(&rule_name)) && rule_name)
                {
                    std::string string_rule_name = ConvertSystemStringToStdString(rule_name.m_str);
                    if (regex_find_match(find_expr, string_rule_name) == true)
                    {
                        vt_wait_delete.push_back(rule_name);
                    }
                }
            }
        }

        for (auto name : vt_wait_delete)
        {
            pRules->Remove(name);
        }
        return true;
    }

    bool IsFireWallEnabled()
    {
        bool bEanbled = false;
        CComPtr<INetFwPolicy2> pPolicy = GetFireWallPolicy();
        if (pPolicy == NULL)
        {
            return false;
        }

        VARIANT_BOOL bVarEnabled = FALSE;
        static NET_FW_PROFILE_TYPE2 FwProFileBitmaskArray[] = {
            NET_FW_PROFILE2_DOMAIN,
            NET_FW_PROFILE2_PRIVATE,
            NET_FW_PROFILE2_PUBLIC
        };

        for (auto bitmask : FwProFileBitmaskArray)
        {
            if (SUCCEEDED(pPolicy->get_FirewallEnabled(bitmask, &bVarEnabled)))
            {
                if (bVarEnabled != FALSE)
                {
                    bEanbled = true;
                    break;
                }
            }
        }
        return bEanbled;
    }
}