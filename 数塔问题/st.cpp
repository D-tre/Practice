//题目链接 https://www.luogu.com.cn/problem/P1216
#include <iostream>
#include <memory>

#define MAXSIZE 1000+3

inline int read() 
{
	int x = 0, f = 1;
	char ch = getchar();
	while (ch < '0' || ch > '9') 
	{
		if (ch == '-')
			f = -1;
		ch = getchar();
	}
	while (ch >= '0' && ch <= '9') 
	{
		x = x * 10 + ch - '0';
		ch = getchar();
	}
	return x * f;
}

int main()
{
	std::ios::sync_with_stdio(false);
	std::cin.tie(0);
	std::cout.tie(0);
	int n=read();
	int nSum =read();
	std::unique_ptr<int> s1(new int[MAXSIZE]);
	std::unique_ptr<int> s2(new int[MAXSIZE]);
	s1.get()[0] = nSum;
	for (int i = 2; i <= n; ++i)
	{
		for (int j = 0; j < i; ++j)
		{
			s2.get()[j]=read();
			if (j == 0)
				s2.get()[j] += s1.get()[j];
			else if (j == i - 1)
				s2.get()[j] += s1.get()[j - 1];
			else
				s2.get()[j] += std::max(s1.get()[j], s1.get()[j - 1]);
			if (s2.get()[j] > nSum)
				nSum = s2.get()[j];
		}
		std::swap(s1, s2);
	}
     std::cout << nSum << std::endl;
	return 0;
}