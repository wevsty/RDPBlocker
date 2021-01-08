#include "block_address_status.h"

block_address_status::block_address_status() 
	:m_count(0),
	m_blocked_timestamp(0),
	m_expire_timestamp(0)
{
}

block_address_status::~block_address_status()
{
}

std::time_t block_address_status::current_timestamp()
{
	return std::time(nullptr);
}

int block_address_status::get_count() const
{
	return m_count;
}

void block_address_status::set_count(const int count)
{
	m_count = count;
}

std::time_t block_address_status::get_expire_interval(const int expire_interval)
{
	return m_expire_timestamp;
}

void block_address_status::set_expire_interval(const int expire_interval)
{
	m_expire_timestamp = current_timestamp() + expire_interval;
}

bool block_address_status::is_expired() const
{
	return current_timestamp() > m_expire_timestamp;
}

std::time_t block_address_status::get_blocked_interval() const
{
	return current_timestamp() - m_blocked_timestamp;
}

void block_address_status::set_blocked_interval(const int block_interval)
{
	m_blocked_timestamp = current_timestamp() + block_interval;
}

void block_address_status::reset_blocked_interval()
{
	m_blocked_timestamp = 0;
}

bool block_address_status::is_blocked() const
{
	return current_timestamp() <= m_blocked_timestamp;
}