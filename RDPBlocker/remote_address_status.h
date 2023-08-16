#ifndef __REMOTE_ADDRESS_STATUS__
#define __REMOTE_ADDRESS_STATUS__

#include <string>
#include <ctime>
#include "timer.h"

class RemoteAddressStatus
{
private:
	int m_count;
	bool m_blocked;
	Timer m_expire_timer;

public:
	RemoteAddressStatus();
	~RemoteAddressStatus();

	int get_count() const;
	void set_count(const int count);

	void reset_expired_timer();
	bool is_expired(const std::time_t value) const;

	bool get_block_flag() const;
	void set_block_flag(const bool flag);
	bool is_blocked() const;
};

#endif // __REMOTE_ADDRESS_STATUS__
