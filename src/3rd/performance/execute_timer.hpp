#include <stdio.h>
#include <chrono>
#include <string>

class execute_timer
{
public:
	execute_timer(const char* print_head)
	{
		m_print_head = print_head;
		m_start_time_stamp = std::chrono::steady_clock::now();
		m_last_time_stamp = std::chrono::steady_clock::now();
	};
	~execute_timer()
	{
		unsigned long long microseconds_delay = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - m_start_time_stamp).count();
		char buffer[1024] = { 0 };
		sprintf(buffer, "\n%s execute total time is %lld mms\n", m_print_head.c_str(), microseconds_delay);
		printf(buffer);
	};
	void print_time()
	{
		unsigned long long microseconds_delay = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - m_last_time_stamp).count();
		m_last_time_stamp = std::chrono::steady_clock::now();
		char buffer[1024] = { 0 };
		sprintf(buffer, "\n%s execute time is %lld mms\n", m_print_head.c_str(), microseconds_delay);
		printf(buffer);
	};

private:
	std::string m_print_head;
	std::chrono::steady_clock::time_point m_start_time_stamp;
	std::chrono::steady_clock::time_point m_last_time_stamp;

};
