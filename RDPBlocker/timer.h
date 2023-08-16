#ifndef __STANDARD_TIMER__
#define __STANDARD_TIMER__

#include <ctime>

class Timer
{
public:
	Timer();
	~Timer();

	void initialize();
	std::time_t elapsed() const;
	bool is_timeout(const std::time_t value) const;

	std::time_t get() const;
	void set(const std::time_t value);

private:
	volatile std::time_t m_time;
};

#endif // __STANDARD_TIMER__