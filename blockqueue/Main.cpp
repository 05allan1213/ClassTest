/////////////////////////////////////////////////////基于阻塞队列的生产消费模型//////////////////////////////////////////////////////////

#include "BlockQueue.hpp"
#include "Task.hpp"
#include <mutex>

std::mutex cout_mutex;
void *consumer(void *args)
{
    BlockQueue<Task> *bq = (static_cast<BlockQueues<Task, Save> *>(args))->c_bq;
    BlockQueue<Save> *save_bq = (static_cast<BlockQueues<Task, Save> *>(args))->s_bq;

    while (true)
    {
        // 消费活动
        Task t;
        bq->pop(&t);
        std::string result = t();
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "cal thread, 完成计算任务: " << result << "... done" << std::endl;
        }
        std::cout.flush();

        // Save _save(result, save);
        // save_bq->push(_save);
        // {
        //     std::lock_guard<std::mutex> lock(cout_mutex);
        //     std::cout << "cal thread, 推送存储任务完成 " << std::endl;
        // }
        //
        // std::cout.flush();

        // sleep(1);
    }
    return nullptr;
}

void *producer(void *args)
{
    BlockQueue<Task> *bq = (static_cast<BlockQueues<Task, Save> *>(args))->c_bq;

    while (true)
    {
        sleep(1);
        // 生产活动
        int x = rand() % 100 + 1;
        int y = rand() % 10;
        int opercode = rand() % oper.size();
        Task t(x, y, oper[opercode], mymath);
        bq->push(t);
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "productor thread, 生产计算任务: " << t.toString() << std::endl;
        }
        std::cout.flush();
    }
    return nullptr;
}

void *saver(void *args)
{
    BlockQueue<Save> *save_bq = (static_cast<BlockQueues<Task, Save> *>(args))->s_bq;
    while (true)
    {
        Save s;
        save_bq->pop(&s);
        s();
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "save thread, 保存任务完成... " << std::endl;
        }
        std::cout.flush();
    }

    return nullptr;
}

int main()
{
    srand((unsigned long)time(nullptr) ^ getpid());

    BlockQueues<Task, Save> bqs;

    bqs.c_bq = new BlockQueue<Task>();
    bqs.s_bq = new BlockQueue<Save>();

    const int num_producers = 3; // 生产者线程数量
    const int num_consumers = 2; // 消费者线程数量

    std::vector<pthread_t> producers(num_producers);
    std::vector<pthread_t> consumers(num_consumers);
    // pthread_t save;

    // 创建生产者线程
    for (int i = 0; i < num_producers; ++i)
    {
        pthread_create(&producers[i], nullptr, producer, &bqs);
    }

    // 等待一段时间，确保生产者线程已经开始生产任务
    sleep(1);

    // 创建消费者线程
    for (int i = 0; i < num_consumers; ++i)
    {
        pthread_create(&consumers[i], nullptr, consumer, &bqs);
    }

    // 创建存储者线程
    // pthread_create(&save, nullptr, saver, &bqs);

    // 等待生产者线程结束
    for (int i = 0; i < num_producers; ++i)
    {
        pthread_join(producers[i], nullptr);
    }

    // 等待消费者线程结束
    for (int i = 0; i < num_consumers; ++i)
    {
        pthread_join(consumers[i], nullptr);
    }

    // 等待存储者线程结束
    // pthread_join(save, nullptr);

    delete bqs.c_bq;
    delete bqs.s_bq;
    return 0;
}