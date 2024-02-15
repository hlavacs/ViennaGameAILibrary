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
		std::chrono::duration<float> duration = std::chrono::high_resolution_clock::now() - m_StartTime; 
		std::cout << m_Name << ": " << duration.count() * 1000.0f << "ms" << std::endl;
	}
private:
	std::string m_Name;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;
};