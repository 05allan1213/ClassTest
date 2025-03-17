/////////////////////////////////////////////////////基于环形队列的生产消费模型//////////////////////////////////////////////////////////

#include "RingQueue.hpp"
#include "Task.hpp"
#include <ctime>
#include <cstdlib>
#include <mutex>
#include <unistd.h>
#include <sys/types.h>

std::mutex cout_mutex;
std::string SelfName()
{
    char name[64];
    snprintf(name, sizeof(name), "thread->0x%x", pthread_self());
    return name;
}
void *producer(void *arg)
{
    RingQueue<Task> *rq = static_cast<RingQueue<Task> *>(arg);

    while (true)
    {
        // version 1
        // sleep(3);
        // int data = rand() % 100;
        // rq->Push(data);
        // std::cout << "producer data: " << data << std::endl;

        // version 2
        int x = rand() % 10;
        int y = rand() % 20;
        char op = oper[rand() % oper.size()];
        Task t(x, y, op, mymath);
        rq->Push(t);
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << SelfName() << ", producer task: " << t.toString() << std::endl;

        // sleep(1);
    }
}

void *consumer(void *arg)
{
    RingQueue<Task> *rq = static_cast<RingQueue<Task> *>(arg);

    while (true)
    {
        // version 1
        // int data;
        // rq->Pop(&data);
        // std::cout << "consumer data: " << data << std::endl;
        // sleep(1);

        // version 2
        Task t;
        rq->Pop(&t);
        std::string result = t();
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << SelfName() << ", consumer task: " << result << std::endl;
    }
}

int main()
{
    srand((unsigned long)time(nullptr) ^ getpid());

    RingQueue<Task> *rq = new RingQueue<Task>();

    const int num_producers = 10; // 生产者线程数量
    const int num_consumers = 15; // 消费者线程数量

    std::vector<pthread_t> producers(num_producers);
    std::vector<pthread_t> consumers(num_consumers);

    for (int i = 0; i < num_producers; i++)
        pthread_create(&producers[i], nullptr, producer, rq);

    for (int i = 0; i < num_consumers; i++)
        pthread_create(&consumers[i], nullptr, consumer, rq);

    for (int i = 0; i < num_producers; i++)
        pthread_join(producers[i], nullptr);

    for (int i = 0; i < num_consumers; i++)
        pthread_join(consumers[i], nullptr);

    delete rq;
    return 0;
}