#include <iostream>
#include <memory>
using namespace std;

#define MAXSIZE 1000+3

int read() 
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
	ios::sync_with_stdio(false);
	cin.tie(0);
	cout.tie(0);
	int n;
	cin >> n;
	int nSum = 0, nTemp;
	cin >> nSum;
	std::unique_ptr<int> s1(new int[MAXSIZE]);
	std::unique_ptr<int> s2(new int[MAXSIZE]);
	s1.get()[0] = nSum;
	for (int i = 2; i <= n; ++i)
	{
		for (int j = 0; j < i; ++j)
		{
			cin >> s2.get()[j];
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
	std::cout << nSum << endl;
	return 0;
}