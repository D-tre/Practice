#pragma once
#include <iostream>
#include <chrono>
#include <string>
class Profiler//��ʱ������
{
public:
	Profiler(const std::string& tag)
	{
		m_Tag = tag;
		m_StartTime = std::chrono::high_resolution_clock::now();
	}
	~Profiler()
	{
		auto endTime = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_StartTime).count();
		std::cout << m_Tag << " ��ʱ: " << duration << " ms" << std::endl;
	}
private:
	std::string m_Tag;
	std::chrono::steady_clock::time_point m_StartTime;
};