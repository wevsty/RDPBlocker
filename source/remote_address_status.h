#ifndef __REMOTE_ADDRESS_STATUS__
#define __REMOTE_ADDRESS_STATUS__

#include <ctime>
#include <string>

#include "timer.h"

class RemoteAddressStatus
{
    private:
    int m_count;
    // 过期时间
    int m_expire_time;
    Timer m_expire_timer;
    // 阻挡时间
    int m_block_time;
    Timer m_blocked_timer;

    public:
    RemoteAddressStatus();
    ~RemoteAddressStatus();

    int get_count() const;
    void set_count(const int count);

    int get_expire_time() const;
    void set_expire_time(const int expired_time);
    void reset_expire_timer();
    bool is_expired() const;

    int get_block_time() const;
    void set_block_time(const int block_time);
    void reset_block_timer();
    bool is_blocked() const;
};

#endif  // __REMOTE_ADDRESS_STATUS__
