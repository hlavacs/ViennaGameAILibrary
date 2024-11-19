/**
* The Vienna Game AI Library
*
* (c) bei Lavinia-Elena Lehaci, University of Vienna, 2024
*
*/

#pragma once
#include <iostream>
#include <chrono>

class Timer
{
public:
	Timer(const std::string& name)
		: m_Name(name)
	{}

	~Timer() {}

	void start()
	{
		m_StartTime = std::chrono::high_resolution_clock::now();
	}

	void end()
	{
		std::chrono::time_point<std::chrono::high_resolution_clock> endTime = std::chrono::high_resolution_clock::now();

		long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTime).time_since_epoch().count();
		long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();

		m_Duration = end - start;
	}

	auto getDuration()
	{
		return m_Duration;
	}

	void print()
	{
		std::cout << m_Name << ": " << getDuration() << " microseconds" << std::endl;
	}

private:
	std::string m_Name;
	long long m_Duration = 0;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;
};