#include "remote_address_status.h"

RemoteAddressStatus::RemoteAddressStatus()
    : m_count(0), m_blocked(false), m_expire_timer()
{
}

RemoteAddressStatus::~RemoteAddressStatus()
{
}

int RemoteAddressStatus::get_count() const
{
    return m_count;
}

void RemoteAddressStatus::set_count(const int count)
{
    m_count = count;
}

void RemoteAddressStatus::reset_expired_timer()
{
    m_expire_timer.initialize();
}

bool RemoteAddressStatus::is_expired(const std::time_t value) const
{
    return m_expire_timer.is_timeout(value);
}

bool RemoteAddressStatus::get_block_flag() const
{
    return m_blocked;
}

void RemoteAddressStatus::set_block_flag(const bool flag)
{
    m_blocked = flag;
}

bool RemoteAddressStatus::is_blocked() const
{
    return m_blocked;
}
