#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <vector>

class singleObj
{
public:
    singleObj()
    {
        m_nSize = 0;
        m_mutex = PTHREAD_MUTEX_INITIALIZER;
    }
    ~singleObj()
    {
    }
    static singleObj *GetInstance()
    {
        static singleObj obj;
        return &obj;
    }
    void syncMethod(bool bAdd)
    {
        pthread_mutex_lock(&m_mutex);
        if (bAdd)
            ++m_nSize;
        else
            --m_nSize;
        pthread_mutex_unlock(&m_mutex);
    }
    const int &getSize() const
    {
        return m_nSize;
    }

private:
    pthread_mutex_t m_mutex;
    int m_nSize;
};

void *threadFunc(void *arg)
{
    bool bAdd = *(bool *)arg;
    singleObj::GetInstance()->syncMethod(bAdd);
    free(arg);
}

int main()
{
    std::vector<pthread_t> vecHandle;
    for (int i = 0; i < 100; ++i)
    {
        pthread_t threadTemp;
        bool *b = (bool *)malloc(sizeof(bool));
        *b = i % 2 == 0;
        pthread_create(&threadTemp, NULL, threadFunc, b);
        vecHandle.push_back(threadTemp);
    }
    for (const auto &handle : vecHandle)
        pthread_join(handle, NULL);
    std::cout << "dataSize:" << singleObj::GetInstance()->getSize() << std::endl;
    return 0;
}