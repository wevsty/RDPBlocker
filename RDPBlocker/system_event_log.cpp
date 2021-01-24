#include "system_event_log.h"

bool SubscribeSystemEvent::SubscribeRDPAuthFailedEvent()
{
	// 查询语句
	// "Event/System[EventID=4625]";
	// "*[System[(EventID=4625) and TimeCreated[timediff(@SystemTime) <= 86400000]]]";
	std::string path = "Security";
	std::string query = "*[System[(EventID=4625) and TimeCreated[timediff(@SystemTime) <= 86400000]]]";
	
	return Subscribe(path, query);
}

bool SubscribeSystemEvent::Subscribe(const std::string& path, const std::string& query)
{
	std::wstring ws_path = boost::locale::conv::to_utf<wchar_t>(path, "UTF-8");
	std::wstring ws_query = boost::locale::conv::to_utf<wchar_t>(query, "UTF-8");

	if (CreateSignal() == false)
	{
		g_logger->error("CreateEvent failed with {}.", GetLastError());
		return false;
	}
	SetSignal();

	// Subscribe to events.
	m_handle_subscription = EvtSubscribe(
		NULL,
		m_handle_signal_event.Get(),
		ws_path.c_str(),
		ws_query.c_str(),
		NULL,
		NULL,
		NULL,
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

DWORD SubscribeSystemEvent::GetEventLogRecordSize(EVT_HANDLE hEvent, DWORD& dwBufferSize)
{
	DWORD dwStatus = ERROR_SUCCESS;
	DWORD dwBufferUsed = 0;
	DWORD dwPropertyCount = 0;
	LPWSTR pRenderedContent = NULL;

	dwBufferSize = 0;

	DWORD bRet = FALSE;
	// The EvtRenderEventXml flag tells EvtRender to render the event as an XML string.
	bRet = EvtRender(NULL, hEvent, EvtRenderEventXml, dwBufferSize, pRenderedContent, &dwBufferUsed, &dwPropertyCount);
	
	if (bRet != TRUE)
	{
		dwStatus = GetLastError();
		if (ERROR_INSUFFICIENT_BUFFER == dwStatus)
		{
			dwBufferSize = dwBufferUsed;
			dwStatus = ERROR_SUCCESS;
		}
	}
	return dwStatus;
}
DWORD SubscribeSystemEvent::GetEventLogRecord(EVT_HANDLE hEvent, std::string& output)
{
	DWORD dwStatus = ERROR_SUCCESS;
	DWORD dwBufferSize = 0;
	DWORD dwBufferUsed = 0;
	DWORD dwPropertyCount = 0;
	LPWSTR pRenderedContent = NULL;

	dwStatus = GetEventLogRecordSize(hEvent, dwBufferSize);
	if (ERROR_SUCCESS != dwStatus)
	{
		g_logger->error("GetEventLogRecordSize failed with {}.", dwStatus);
		return dwStatus;
	}

	std::shared_ptr<WCHAR> buffer(static_cast<WCHAR*>(malloc(dwBufferSize)), free);
	pRenderedContent = buffer.get();
	if (pRenderedContent == NULL)
	{
		dwStatus = ERROR_OUTOFMEMORY;
		return dwStatus;
	}

	DWORD bRet = FALSE;
	// The EvtRenderEventXml flag tells EvtRender to render the event as an XML string.
	bRet = EvtRender(NULL, hEvent, EvtRenderEventXml, dwBufferSize, pRenderedContent, &dwBufferUsed, &dwPropertyCount);
	if (bRet != TRUE)
	{
		dwStatus = GetLastError();
		g_logger->error("EvtRender failed with {}.", dwStatus);
		return dwStatus;
	}
	output = boost::locale::conv::utf_to_utf<char>(pRenderedContent);

	return dwStatus;
}

bool SubscribeSystemEvent::StoreEventLogResults(std::vector<std::string>& vt_results)
{
	#define EVENTS_ARRAY_SIZE 12

	while (true)
	{
		DWORD status = ERROR_SUCCESS;
		EVT_HANDLE hEvents[EVENTS_ARRAY_SIZE] = {NULL};
		DWORD dwReturned = 0;
		std::vector<EventLogHandleWrapper*> vt_handle;
		// Get a block of events from the result set.
		status = EvtNext(m_handle_subscription, EVENTS_ARRAY_SIZE, hEvents, INFINITE, 0, &dwReturned);
		if (status != TRUE)
		{
			status = GetLastError();
			if (ERROR_NO_MORE_ITEMS != status)
			{
				g_logger->error("EvtNext failed with {}.", status);
				return false;
			}
			return true;
		}
		for (DWORD i = 0; i < dwReturned; i++)
		{
			std::string record_buffer;
			status = GetEventLogRecord(hEvents[i], record_buffer);
			if (ERROR_SUCCESS == status)
			{
				vt_results.push_back(record_buffer);
				EventLogHandleWrapper* p_wrapper = new EventLogHandleWrapper(hEvents[i]);
				vt_handle.push_back(p_wrapper);
				hEvents[i] = NULL;
			}
			else
			{
				break;
			}
		}
	}
	return true;
}

bool SubscribeSystemEvent::CreateSignal()
{
	m_handle_signal_event = CreateEventW(NULL, TRUE, FALSE, NULL);
	if (m_handle_signal_event.IsInvalid())
	{
		g_logger->error("CreateEvent failed with {}.", GetLastError());
		return false;
	}
	return true;
}

bool SubscribeSystemEvent::WaitSignal()
{
	DWORD dwRet = WaitForSingleObject(m_handle_signal_event, INFINITE);
	if (dwRet == WAIT_OBJECT_0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SubscribeSystemEvent::SetSignal()
{
	if (SetEvent(m_handle_signal_event) == FALSE)
	{
		return false;
	}
	return true;
}

bool SubscribeSystemEvent::ResetSignal()
{
	if (ResetEvent(m_handle_signal_event) == FALSE)
	{
		return false;
	}
	return true;
}

bool GetLogEventDataToMap(const std::string& xml_data,std::map<std::string, std::string>& attr)
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