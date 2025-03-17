#pragma once

#include <iostream>
#include <vector>
#include <semaphore.h>
#include <pthread.h>

static const int MAX_SIZE = 10;

template <class T>
class RingQueue
{
public:
    RingQueue(const int &size = MAX_SIZE) : _queue(size), _size(size), pro_step(0), con_step(0)
    {
        sem_init(&_psem, 0, MAX_SIZE);
        sem_init(&_csem, 0, 0);

        pthread_mutex_init(&_pmutex, nullptr);
        pthread_mutex_init(&_cmutex, nullptr);
    }

    void Push(const T &in)
    {
        P(_psem);
        pthread_mutex_lock(&_pmutex);
        _queue[pro_step++] = in;
        pro_step = pro_step % _size;
        pthread_mutex_unlock(&_pmutex);
        V(_csem);
    }

    void Pop(T *out)
    {
        P(_csem);
        pthread_mutex_lock(&_cmutex);
        *out = _queue[con_step++];
        con_step = con_step % _size;
        pthread_mutex_unlock(&_cmutex);
        V(_psem);
    }
    ~RingQueue()
    {
        sem_destroy(&_psem);
        sem_destroy(&_csem);

        pthread_mutex_destroy(&_pmutex);
        pthread_mutex_destroy(&_cmutex);
    }

private:
    void P(sem_t &sem)
    {
        sem_wait(&sem);
    }

    void V(sem_t &sem)
    {
        sem_post(&sem);
    }

private:
    std::vector<T> _queue;
    int _size;
    sem_t _psem; // 生产者想生产，看重的是空间资源
    sem_t _csem; // 消费者想消费，看重的是数据资源
    int pro_step;
    int con_step;
    pthread_mutex_t _pmutex;
    pthread_mutex_t _cmutex;
};
