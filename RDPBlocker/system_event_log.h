#ifndef __SYSTEM_EVENT_LOG_CLASS__
#define __SYSTEM_EVENT_LOG_CLASS__

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <boost/locale.hpp>
#include "logger.h"
#include "handle_wrapper.h"
#include "boost_xml_utils.h"

#include <windows.h>
#include <winevt.h>

#pragma comment(lib, "wevtapi.lib")


template <typename HANDLE>
class EventLogHandleTraits
{
public:
    static constexpr HANDLE InvalidValue() {
        return NULL;
    }

    static void Close(HANDLE value)
    {
        EvtClose(value);
    }
};
typedef HandleWrapper<EVT_HANDLE, EventLogHandleTraits<EVT_HANDLE> > EventLogHandleWrapper;


class SubscribeSystemEvent
{
public:
    EventLogHandleWrapper m_handle_subscription;
    NullKernelHandleWrapper m_handle_signal_event;

    // 订阅RDP登录失败事件
    bool SubscribeRDPAuthFailedEvent();
    // 订阅系统事件
    bool Subscribe(const std::string& path, const std::string& query);
    // 获取事件记录所需要的缓冲区大小
    DWORD GetEventLogRecordSize(EVT_HANDLE hEvent, DWORD& dwBufferSize);
    // 获取事件记录
    DWORD GetEventLogRecord(EVT_HANDLE hEvent, std::string& output);
    // 获取多条记录并储存到vector
    bool StoreEventLogResults(std::vector<std::string>& vt_results);
    // 创建信号
    bool CreateSignal();
    // 等待信号
    bool WaitSignal();
    // 设置信号
    bool SetSignal();
    // 重置信号
    bool ResetSignal();
};

// 把XML数据中的Event.EventData项下的所有项目添加到map
bool GetLogEventDataToMap(const std::string& xml_data, std::map<std::string, std::string>& attr);
#endif //__SYSTEM_EVENT_LOG_CLASS__