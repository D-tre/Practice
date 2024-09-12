#include <iostream>
#include <vector>

using namespace std;


class Solution 
{
public:
    /**
     * @param m: An integer m denotes the size of a backpack
     * @param a: Given n items with size A[i]
     * @param v: Given n items with value V[i]
     * @return: The maximum value
     */
    int backPackII(int m, vector<int>& a, vector<int>& v) {
        int dp[103][1003];
        dp[0][0] = 0;
        for (int i = 1; i <= a.size(); ++i)
        {
            for (int j = 1; j <= m; ++j)
            {
                if (j >= a[i - 1])
                    dp[i][j] = max(dp[i - 1][j], dp[i - 1][j - a[i - 1]] + v[i - 1]);
                else
                {
                    dp[i][j] = dp[i - 1][j];
                }
            }
        }
        return dp[a.size()][m];
    }
};

//int main()
//{ 
//	ios::sync_with_stdio(false);
//	cin.tie(0);
//	cout.tie(0);
//
//	dp[0][0] = 0;
//}