#include "timer.h"

Timer::Timer() : m_time(time(0))
{
}

Timer::~Timer()
{
}

void Timer::initialize()
{
    m_time = time(0);
}

std::time_t Timer::elapsed() const
{
    return std::time(0) - m_time;
}

bool Timer::is_timeout(const std::time_t value) const
{
    return elapsed() > value;
}

std::time_t Timer::get() const
{
    return m_time;
}

void Timer::set(const std::time_t value)
{
    m_time = value;
}

void Timer::reset()
{
    m_time = time(0);
}
