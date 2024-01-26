// ConsoleApplication1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include<unordered_map>
#include <vector>
#include <map>
#include "CalTime.hpp"

using namespace std;

int main()
{
	std::map<std::string, std::string> map;
	std::map<std::string, std::string> map1;
	std::map<std::string, std::string> map2;
	std::map<std::string, std::string> map3;
	for (int i = 0; i < 1000000; ++i)
	{
		map.insert(std::make_pair("11111", ",,,,,,"));
	}
	{
		Profiler timeCal("map拷贝");
		for (const auto& pairData : map)
		{
			map1.insert(std::make_pair(pairData.first, pairData.second));
		}
	}
	//{
	//	Profiler timeCal("map拷贝/移动耗时");
	//	for (const auto& pairData : map1)
	//	{
	//		map2.insert(std::move(std::make_pair(pairData.first, pairData.second)));
	//	}
	//}
	//{
	//	Profiler timeCal("map移动耗时");
	//	for (const auto& pairData : map2)
	//	{
	//		map3.insert(std::move(pairData.first),std::move( pairData.second));
	//	}
	//}
}