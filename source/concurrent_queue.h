#ifndef __CONCURRENT_QUEUE__
#define __CONCURRENT_QUEUE__

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <queue>

template <typename T_DATA>
class concurrent_queue
{
    private:
    std::queue<T_DATA> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_condition_variable;

    public:
    void push(T_DATA const& value)
    {
        std::unique_lock lock(m_mutex);
        m_queue.push(value);
        lock.unlock();
        m_condition_variable.notify_one();
    }

    void pop(T_DATA& value)
    {
        wait_pop(value);
    }

    bool empty() const
    {
        std::unique_lock lock(m_mutex);
        return m_queue.empty();
    }

    std::size_t size() const
    {
        std::unique_lock lock(m_mutex);
        return m_queue.size();
    }

    bool try_pop(T_DATA& value)
    {
        std::unique_lock lock(m_mutex);
        if (m_queue.empty())
        {
            return false;
        }

        value = m_queue.front();
        m_queue.pop();
        return true;
    }

    void wait_pop(T_DATA& value)
    {
        std::unique_lock lock(m_mutex);
        while (m_queue.empty())
        {
            m_condition_variable.wait(lock);
        }

        value = m_queue.front();
        m_queue.pop();
    }
};

#endif