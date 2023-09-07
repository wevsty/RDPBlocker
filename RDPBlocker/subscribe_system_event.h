#ifndef __SUBSCRIBE_SYSTEM_EVENT_CLASS__
#define __SUBSCRIBE_SYSTEM_EVENT_CLASS__

#include <map>
#include <memory>
#include <string>

#include <windows.h>
#include <winevt.h>

#include "boost_xml_utils.h"
#include "concurrent_queue.h"
#include "handle_wrapper.h"
#include "logger.h"
#include "utf_convert.h"

#pragma comment(lib, "wevtapi.lib")

template <typename HANDLE>
class EventLogHandleTraits
{
    public:
    static constexpr HANDLE InvalidValue()
    {
        return NULL;
    }

    static void Close(HANDLE value)
    {
        EvtClose(value);
    }
};
typedef HandleWrapper<EVT_HANDLE, EventLogHandleTraits<EVT_HANDLE> >
    EventLogHandleWrapper;

class SubscribeSystemEventBase
{
    public:
    EventLogHandleWrapper m_handle_subscription;
    concurrent_queue<std::string> m_queue;
    std::string m_path;
    std::string m_query;

    SubscribeSystemEventBase();
    ~SubscribeSystemEventBase();

    static DWORD WINAPI SubscriptionCallback(EVT_SUBSCRIBE_NOTIFY_ACTION action,
                                             PVOID pContext,
                                             EVT_HANDLE hEvent);
    static DWORD GetEventData(EVT_HANDLE hEvent, std::wstring& out);

    bool Subscribe();
    bool Subscribe(const std::string& path, const std::string& query);
    void Push(std::string const& data);
    void Pop(std::string& data);
};

class RDPAuthFailedEvent : public SubscribeSystemEventBase
{
    public:
    RDPAuthFailedEvent();
    ~RDPAuthFailedEvent();
};

class RDPAuthSucceedEvent : public SubscribeSystemEventBase
{
    public:
    RDPAuthSucceedEvent();
    ~RDPAuthSucceedEvent();
};

bool EventDataToMap(const std::string& xml_data,
                    std::map<std::string, std::string>& attr);

#endif
