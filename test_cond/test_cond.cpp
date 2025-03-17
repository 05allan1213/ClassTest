#include <iostream>
#include <pthread.h>
#include <unistd.h>

using namespace std;

// 互斥锁和条件变量都定义为自动初始化和释放
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

const int num = 5; // 创建五个线程

void *Active(void *args)
{
    const char *name = static_cast<const char *>(args);

    while (true)
    {
        // 加锁
        pthread_mutex_lock(&mtx);

        // 等待条件满足
        pthread_cond_wait(&cond, &mtx);
        cout << "\t线程 " << name << " 正在运行" << endl;

        // 解锁
        pthread_mutex_unlock(&mtx);
    }

    delete[] name;
    return nullptr;
}

int main()
{
    pthread_t pt[num];
    for (int i = 1; i <= num; i++)
    {
        char *name = new char[32];
        snprintf(name, 32, "thread-%d", i);
        pthread_create(pt + i, nullptr, Active, name);
    }

    // 等待所有次线程就位
    sleep(3);

    // 主线程唤醒次线程
    while (true)
    {
        cout << "Main thread wake up Other thread!" << endl;
        // pthread_cond_signal(&cond); // 单个唤醒
        pthread_cond_broadcast(&cond); // 全部唤醒
        sleep(1);
    }

    for (int i = 0; i < num; i++)
        pthread_join(pt[i], nullptr);

    return 0;
}
