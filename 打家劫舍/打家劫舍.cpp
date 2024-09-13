#include <iostream>
#include <vector>

using namespace std;
class Solution 
{
public:
    Solution()
    {
        ios::sync_with_stdio(false);
        cin.tie(0);
        cout.tie(0);
    }
    int rob(vector<int>& nums) 
    {
        const auto &nSize = nums.size();
        if(nSize==1)
            return nums[0];
        if(nSize==2)
            return std::max(nums[0], nums[1]);
        nums[2] += nums[0];
        for (int i = 3; i < nSize;++i)
        {
            nums[i] += std::max(nums[i - 2], nums[i - 3]);
        }
        return std::max(nums[nSize - 1], nums[nSize - 2]);
    }
};