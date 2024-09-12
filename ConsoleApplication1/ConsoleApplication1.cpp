#include <iostream>
#include <vector>


class A
{
public:
    A(int a)
        :mA(a)
    {
        int i = 0;
    }
    int mA;
};
int main()
{
    std::vector<A> vecA;
    A a(1);
    vecA.push_back(a);
    return 0;
}