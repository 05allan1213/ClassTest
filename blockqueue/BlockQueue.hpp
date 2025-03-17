#pragma once

#include <iostream>
#include <string>
#include <queue>
#include <ctime>
#include <unistd.h>
#include <pthread.h>

static const int MAX_SIZE = 50;

template <class T>
class BlockQueue
{
public:
    BlockQueue(const int &size = MAX_SIZE)
        : _size(size)
    {
        pthread_mutex_init(&_mutex, nullptr);
        pthread_cond_init(&_pcond, nullptr);
        pthread_cond_init(&_ccond, nullptr);
    }

    void push(const T &in) // 输入型参数：const &
    {
        pthread_mutex_lock(&_mutex);
        // 1. 判断队列是否满
        while (isfull()) // 循环判断，直到队列不满才退出循环
        {
            pthread_cond_wait(&_pcond, &_mutex); // 因为生产条件不满足，此时生产者阻塞等待
        }
        // 2. 添加元素
        _queue.push(in);
        // 3. 通知消费者
        pthread_cond_signal(&_ccond);

        pthread_mutex_unlock(&_mutex);
    }

    void pop(T *out) // 输出型参数：*   // 输入输出型参数：&
    {
        pthread_mutex_lock(&_mutex);
        // 1. 判断队列是否空
        while (isempty()) // 循环判断，直到队列不空才退出循环
        {
            pthread_cond_wait(&_ccond, &_mutex); // 因为消费者条件不满足，此时消费者阻塞等待
        }
        // 2. 取出元素
        *out = _queue.front();
        _queue.pop();
        // 3. 通知生产者
        pthread_cond_signal(&_pcond);

        pthread_mutex_unlock(&_mutex);
    }

    ~BlockQueue()
    {
        pthread_mutex_destroy(&_mutex);
        pthread_cond_destroy(&_pcond);
        pthread_cond_destroy(&_ccond);
    }

private:
    bool isempty()
    {
        return _queue.empty();
    }

    bool isfull()
    {
        return _queue.size() >= _size;
    }

private:
    std::queue<T> _queue;
    int _size; // 队列元素上限
    pthread_mutex_t _mutex;
    pthread_cond_t _pcond; // 生产者条件变量
    pthread_cond_t _ccond; // 消费者条件变量
};

// C：计算
// S：存储
template <class C, class S>
class BlockQueues
{
public:
    BlockQueue<C> *c_bq;
    BlockQueue<S> *s_bq;
};