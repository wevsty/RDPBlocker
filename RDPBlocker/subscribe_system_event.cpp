#include "subscribe_system_event.h"

SubscribeSystemEventBase::SubscribeSystemEventBase()
{
}

SubscribeSystemEventBase::~SubscribeSystemEventBase()
{
}

DWORD __stdcall SubscribeSystemEventBase::SubscriptionCallback(EVT_SUBSCRIBE_NOTIFY_ACTION action, PVOID pContext, EVT_HANDLE hEvent)
{
    UNREFERENCED_PARAMETER(pContext);

    DWORD status = ERROR_SUCCESS;
    SubscribeSystemEventBase* ptr = static_cast<SubscribeSystemEventBase*>(pContext);
    assert(ptr != NULL);
    switch (action)
    {
    case EvtSubscribeActionDeliver:
    {
        std::wstring event_data;
        status = GetEventData(hEvent, event_data);
        if (ERROR_SUCCESS == status)
        {
            std::string data = boost::locale::conv::utf_to_utf<char>(event_data);
            ptr->Push(data);
        }
        break;
    }
    default:
    {
        g_logger->info("SubscriptionCallback: Unknown action.");
        break;
    }
    }
    // The service ignores the returned status.
    return ERROR_SUCCESS;
}

// 获取订阅事件的XML数据
DWORD SubscribeSystemEventBase::GetEventData(EVT_HANDLE hEvent, std::wstring& out)
{
    DWORD status = ERROR_SUCCESS;
    DWORD dwBufferSize = 0;
    DWORD dwBufferUsed = 0;
    DWORD dwPropertyCount = 0;
    LPWSTR pRenderedContent = NULL;

    if (!EvtRender(NULL, hEvent, EvtRenderEventXml, dwBufferSize, pRenderedContent, &dwBufferUsed, &dwPropertyCount))
    {
        status = GetLastError();
        if (ERROR_INSUFFICIENT_BUFFER == status)
        {
            dwBufferSize = dwBufferUsed;
            std::shared_ptr<unsigned char[]> buffer = std::make_shared<unsigned char[]>(dwBufferSize);
            pRenderedContent = reinterpret_cast<LPWSTR>(buffer.get());
            EvtRender(NULL, hEvent, EvtRenderEventXml, dwBufferSize, pRenderedContent, &dwBufferUsed, &dwPropertyCount);
            status = GetLastError();
            if (ERROR_SUCCESS != status)
            {
                g_logger->error("EvtRender failed with {}.", status);
            }
            out = pRenderedContent;
        }
    }
    return status;
}

bool SubscribeSystemEventBase::Subscribe()
{
    return Subscribe(m_path, m_query);
}

bool SubscribeSystemEventBase::Subscribe(const std::string& path, const std::string& query)
{
    std::wstring ws_path = boost::locale::conv::utf_to_utf<wchar_t>(path);
    std::wstring ws_query = boost::locale::conv::utf_to_utf<wchar_t>(query);

    // Subscribe to events.
    m_handle_subscription = EvtSubscribe(
        NULL,
        NULL,
        ws_path.c_str(),
        ws_query.c_str(),
        NULL,
        this,
        &SubscriptionCallback,
        EvtSubscribeToFutureEvents
    );
    if (m_handle_subscription.IsInvalid())
    {
        DWORD status = GetLastError();

        if (ERROR_EVT_CHANNEL_NOT_FOUND == status)
        {
            g_logger->error("Channel was not found.");
        }
        else if (ERROR_EVT_INVALID_QUERY == status)
        {
            g_logger->error("The query was not found.");
        }
        else
        {
            g_logger->error("EvtSubscribe failed with {}.", status);
        }
        return false;
    }
    return true;
}

void SubscribeSystemEventBase::Push(std::string const& data)
{
    m_queue.push(data);
}

void SubscribeSystemEventBase::Pop(std::string& data)
{
    m_queue.pop(data);
}

RDPAuthFailedEvent::RDPAuthFailedEvent(): SubscribeSystemEventBase()
{
    // 查询语句
    // "Event/System[EventID=4625]";
    // "*[System[(EventID=4625) and TimeCreated[timediff(@SystemTime) <= 86400000]]]";
    m_path = "Security";
    m_query = "*[System[(EventID=4625) and TimeCreated[timediff(@SystemTime) <= 86400000]]]";
}

RDPAuthFailedEvent::~RDPAuthFailedEvent()
{

}

RDPAuthSucceedEvent::RDPAuthSucceedEvent() : SubscribeSystemEventBase()
{
    // 查询语句
    // "Event/System[EventID=4624]";
    // "*[System[(EventID=4624) and TimeCreated[timediff(@SystemTime) <= 86400000]]]";
    m_path = "Security";
    m_query = "*[System[(EventID=4624) and TimeCreated[timediff(@SystemTime) <= 86400000]]]";
}

RDPAuthSucceedEvent::~RDPAuthSucceedEvent()
{

}


bool EventDataToMap(const std::string& xml_data, std::map<std::string, std::string>& attr)
{
    const boost::property_tree::ptree pt = read_xml_from_string(xml_data);
    const boost::property_tree::ptree sub_data_pt = pt.get_child("Event.EventData");

    for (boost::property_tree::ptree::value_type attr_name : sub_data_pt.get_child(""))
    {
        const std::string map_value = attr_name.second.data();
        for (boost::property_tree::ptree::value_type attr_value : attr_name.second.get_child("<xmlattr>"))
        {
            const std::string map_key = attr_value.second.data();
            attr[map_key] = map_value;
        }
    }
    return true;
}

