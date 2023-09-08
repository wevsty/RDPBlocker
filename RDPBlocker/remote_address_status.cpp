#include "remote_address_status.h"

RemoteAddressStatus::RemoteAddressStatus()
    : m_count(0),
      m_expire_time(0),
      m_expire_timer(),
      m_block_time(0),
      m_blocked_timer()
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

int RemoteAddressStatus::get_expire_time() const
{
    return m_block_time;
}

void RemoteAddressStatus::set_expire_time(const int expired_time)
{
    m_expire_time = expired_time;
    m_expire_timer.initialize();
}

void RemoteAddressStatus::reset_expire_timer()
{
    m_expire_timer.reset();
}

bool RemoteAddressStatus::is_expired() const
{
    if (m_expire_time <= 0)
    {
        return false;
    }
    return m_expire_timer.is_timeout(m_expire_time);
}

int RemoteAddressStatus::get_block_time() const
{
    return m_block_time;
}

void RemoteAddressStatus::set_block_time(const int block_time)
{
    m_block_time = block_time;
    m_blocked_timer.initialize();
}

void RemoteAddressStatus::reset_block_timer()
{
    m_blocked_timer.reset();
}

bool RemoteAddressStatus::is_blocked() const
{
    bool b_block_status = false;
    if (m_block_time <= 0)
    {
        return b_block_status;
    }
    b_block_status = !m_blocked_timer.is_timeout(m_block_time);
    return b_block_status;
}
