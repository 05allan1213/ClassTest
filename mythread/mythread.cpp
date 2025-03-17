#include "mythread.hpp"
#include "mutex.hpp"

void *thread_run(void *args)
{
    std::string work_type = static_cast<const char *>(args);
    while (true)
    {
        printf("我是一个新线程，我正在做：%s\n", work_type.c_str());
        sleep(1);
    }
}
int main()
{
    std::unique_ptr<Thread> thread1(new Thread(thread_run, (void *)"线程1", 1));
    std::unique_ptr<Thread> thread2(new Thread(thread_run, (void *)"线程2", 2));
    std::unique_ptr<Thread> thread3(new Thread(thread_run, (void *)"线程3", 3));

    thread1->start();
    thread2->start();
    thread3->start();

    thread1->join();
    thread2->join();
    thread3->join();

    return 0;
}