#include "CalTime.hpp"
#include<vector>
using namespace std;
class MyClass
{
public:
    MyClass(){}
    ~MyClass(){}
    MyClass(MyClass&& other)
    {
        m_arry=other.m_arry;
    }
    MyClass(MyClass& other)
    {
        m_arry=other.m_arry;
    }
    void initArry()
    {
        for(int i=0;i<1000000;++i)
            m_arry.push_back(i);
    }
    MyClass& operator = (MyClass&& other)
    {
        m_arry=other.m_arry;
        return *this;
    }
    std::vector<int> m_arry;
};
int main()
{
    MyClass s1;
    s1.initArry();
    {
        auto profiler = Profiler("construct move");
        for(int i=0;i<10000;++i)
        {
            MyClass d1=std::move(s1);
        }
    }
    MyClass s2;
    s2.initArry();
    {
        auto profiler = Profiler("construct copy");
        for(int i=0;i<10000;++i)
        {
            MyClass d2=std::move(s2);
        }
    }
    MyClass s3;
    s3.initArry();
    {
        MyClass d3;
        auto profiler = Profiler("operator move");
        for(int i=0;i<10000;++i)
        {
            d3=std::move(s3);
            s3=std::move(d3);
        }
    }
    return 0;
}