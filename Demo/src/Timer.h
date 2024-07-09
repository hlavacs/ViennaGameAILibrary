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
	{
		m_StartTime = std::chrono::high_resolution_clock::now();
	}
	~Timer() {
		auto endTime = std::chrono::high_resolution_clock::now();
		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTime).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();

		auto duration = (end - start) * 0.001;
		std::cout << m_Name << ": " << duration << " ms" << std::endl;
	}
private:
	std::string m_Name;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;
};