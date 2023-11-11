#include <string.h>
#include <sstream>
#include <stdio.h>
#include <vector>
#include "CalTime.hpp"
using namespace std;

template<typename T>
void moveData(T& dst, T& src)
{
	dst = std::move(src);
}

template<typename T>
void moveData(T& dst,T&& src)
{
	dst = src;
}

template<typename T>
void copyData(T& dst, T& src)
{
	dst = src;
}
int main()
{
	vector<int> d1;
	{//移动语义代码块
		vector<int> s1(100000);
		auto profiler = Profiler("move");
		for (int i = 0; i < 10000; ++i)
		{
			moveData(d1, s1);
			moveData(s1, d1);
		}
	}
	vector<int> d11;
	{//移动语义代码块
		vector<int> s11(100000);
		auto profiler = Profiler("fail move");
		for (int i = 0; i < 1000; ++i)
		{
			moveData(d11, std::move(s11));
			moveData(s11, std::move(d11));
		}
	}
	vector<int> d2;
	{//拷贝语义代码块
		vector<int> s2(100000);
		auto profiler = Profiler("copy");
		for (int i = 0; i < 1000; ++i)
		{
			copyData(d2, s2);
			copyData(s2, d2);
		}
	}
}