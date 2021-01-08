#ifndef __BLOCK_ADDRESS_STATUS__
#define __BLOCK_ADDRESS_STATUS__

#include <string>
#include <ctime>

class block_address_status
{
public:
	int m_count;
	std::time_t m_blocked_timestamp;
	std::time_t m_expire_timestamp;

	static std::time_t current_timestamp();

	block_address_status();
	~block_address_status();

	int get_count() const;
	void set_count(const int count);

	std::time_t get_expire_interval(const int expire_interval);
	void set_expire_interval(const int expire_interval);
	bool is_expired() const;

	std::time_t get_blocked_interval() const;
	void set_blocked_interval(const int block_interval);
	void reset_blocked_interval();
	bool is_blocked() const;
};

#endif // __BLOCK_ADDRESS_STATUS__
