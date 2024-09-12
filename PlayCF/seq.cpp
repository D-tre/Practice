#include <iostream>
using namespace std;

#define MAXSIZE 1000+3


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
			cin >> nTemp;
			if (j == 0)
				s2.get()[j] = s1.get()[j];
			else if (j == n - 1)
				s2.get()[j] = s1.get()[j - 1];
			else
				s2.get()[j] = std::max(s1.get()[j], s1.get()[j - 1]);
			if (s2.get()[j] > nSum)
				nSum = s2.get()[j];
		}
		std::swap(s1, s2);
	}
}