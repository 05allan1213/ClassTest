// ThreadPool.hpp: 实现了线程池类 ThreadPool，用于管理和调度多个线程执行任务。
//                提供了任务队列、线程管理、任务调度和线程池生命周期管理等功能。
#pragma once

#include "Thread.hpp"
#include "LockGuard.hpp"
#include "Task.hpp"
#include <vector>
#include <queue>
#include <mutex>
#include <iostream>
#include <unistd.h>
#include <sstream> // 引入 stringstream，用于格式化日志信息

using namespace MyThread;

const int MAX_SIZE = 5; // 默认线程池大小

template <class T>
class ThreadPool; // 前向声明 ThreadPool 类

template <class T>
class ThreadData // 线程数据结构，用于传递给工作线程
{
public:
    ThreadPool<T> *threadpool; // 指向所属线程池的指针
    std::string name;          // 线程名称

public:
    ThreadData(ThreadPool<T> *tp, std::string n) : threadpool(tp), name(n)
    {
    }
};

template <class T>
class ThreadPool // 线程池类
{
private:
    // 线程处理任务的静态函数，所有工作线程都执行此函数
    static void *handlerTask(void *args)
    {
        ThreadData<T> *thread_data = (ThreadData<T> *)args; // 获取线程数据
        // 线程循环处理任务，直到线程池被停止
        while (true)
        {
            T task; // 存储取出的任务
            {
                // 使用 LockGuard 自动加解锁互斥锁，保证线程安全访问任务队列
                LockGuard lockguard(thread_data->threadpool->mutex());

                // 检查线程池是否需要停止
                if (thread_data->threadpool->isShutdown())
                {
                    { // 使用 CoutGuard 保护 std::cout，防止多线程输出混乱
                        CoutGuard coutGuard;
                        std::cout << thread_data->name << " 线程收到停止信号，准备退出..." << std::endl;
                    }
                    break; // 退出循环，线程结束
                }

                // 如果任务队列为空，则等待条件变量，释放互斥锁，让出 CPU
                while (thread_data->threadpool->isempty())
                {
                    thread_data->threadpool->threadWait(); // 等待条件变量，线程进入等待状态，并释放互斥锁
                    // 被唤醒后，需要再次检查是否需要停止线程池，避免虚假唤醒后继续执行任务
                    if (thread_data->threadpool->isShutdown())
                    {
                        { // 使用 CoutGuard 保护 std::cout
                            CoutGuard coutGuard;
                            std::cout << thread_data->name << " 线程被唤醒后发现停止信号，准备退出..." << std::endl;
                        }
                        break;
                    }
                }
                // 再次检查是否需要停止，因为可能在等待过程中被设置为 shutdown 状态
                if (thread_data->threadpool->isShutdown())
                {
                    { // 使用 CoutGuard 保护 std::cout
                        CoutGuard coutGuard;
                        std::cout << thread_data->name << " 线程检查到停止信号，准备退出..." << std::endl;
                    }
                    break;
                }

                // 从任务队列中取出一个任务
                task = thread_data->threadpool->pop(); // 从任务队列头部取出任务
            }
            std::string result_str;
            { // 使用 CoutGuard 保护 std::cout
                CoutGuard coutGuard;
                std::cout << thread_data->name << " 线程开始执行任务: " << task.toString() << std::endl;                              // 输出任务开始执行信息
                result_str = task();                                                                                                  // 执行任务，并获取结果字符串 (或日志)
                std::cout << thread_data->name << " 线程完成任务: " << task.toString() << "，结果/日志: " << result_str << std::endl; // 输出任务完成信息和结果
            }

            // 自动写入日志
            std::stringstream log_stream;
            log_stream << thread_data->name << " 完成任务: " << task.toString() << "，结果: " << result_str;
            save(log_stream.str()); // 调用 save 函数将日志写入文件
        }
        delete thread_data; // 释放 ThreadData 的内存
        {                   // 使用 CoutGuard 保护 std::cout
            CoutGuard coutGuard;
            std::cout << thread_data->name << " 线程退出." << std::endl; // 输出线程退出信息
        }
        pthread_exit(nullptr); // 更规范的线程退出方式，避免资源泄露
        return nullptr;
    }

    // 私有构造函数，保证单例模式
    ThreadPool(const int &num = MAX_SIZE)
        : _num(num), _is_shutdown(false) // 初始化线程数量和停止标志
    {
        pthread_mutex_init(&_mutex, nullptr); // 初始化互斥锁，用于保护任务队列
        pthread_cond_init(&_cond, nullptr);   // 初始化条件变量，用于线程等待任务
        for (int i = 0; i < _num; ++i)
        {
            _threads.push_back(new Thread()); // 创建指定数量的线程对象，但此时线程尚未启动
        }
        { // 使用 CoutGuard 保护 std::cout
            CoutGuard coutGuard;
            std::cout << "ThreadPool 初始化完成，线程数量: " << _num << std::endl; // 输出线程池初始化完成信息
        }
    }

    // 禁用拷贝构造函数和拷贝赋值运算符，防止外部拷贝线程池单例
    void operator=(const ThreadPool &) = delete;
    ThreadPool(const ThreadPool &) = delete;

public:
    // 获取互斥锁，外部一般不需要直接调用，使用 LockGuard RAII 风格的锁管理
    void lockQueue()
    {
        pthread_mutex_lock(&_mutex);
    }

    // 解锁互斥锁，外部一般不需要直接调用，使用 LockGuard
    void unlockQueue()
    {
        pthread_mutex_unlock(&_mutex);
    }

    // 判断任务队列是否为空
    bool isempty()
    {
        return _task_queue.empty();
    }

    // 线程等待条件变量，需要先持有互斥锁
    void threadWait()
    {
        pthread_cond_wait(&_cond, &_mutex); // 等待条件变量，线程挂起，原子性地释放互斥锁
    }

    // 从任务队列头部弹出一个任务
    T pop()
    {
        T task = _task_queue.front(); // 获取队首任务
        _task_queue.pop();            // 移除队首任务
        return task;                  // 返回取出的任务
    }

    // 获取互斥锁指针，供 LockGuard 使用
    pthread_mutex_t *mutex()
    {
        return &_mutex;
    }

    // 获取线程池是否被停止的状态
    bool isShutdown() const
    {
        return _is_shutdown;
    }

public:
    // 启动线程池，创建并启动所有工作线程
    void run()
    {
        for (const auto &t : _threads)
        {
            ThreadData<T> *thread_data = new ThreadData<T>(this, t->threadname()); // 为每个线程创建线程数据
            t->start(handlerTask, thread_data);                                    // 启动线程，指定 handlerTask 为线程执行函数
            {                                                                      // 使用 CoutGuard 保护 std::cout
                CoutGuard coutGuard;
                std::cout << t->threadname() << " 线程启动..." << std::endl; // 输出线程启动信息
            }
        }
        { // 使用 CoutGuard coutGuard;
            CoutGuard coutGuard;
            std::cout << "ThreadPool 运行中..." << std::endl; // 输出线程池开始运行信息
        }
        // 在所有线程启动后，稍微等待一下，让线程有机会输出 "任务队列为空，等待新任务..." 信息
        usleep(100000); // 等待 100 毫秒 (0.1 秒)
    }

    // 向任务队列中添加任务
    void Push(const T &in)
    {
        LockGuard lockguard(&_mutex); // 使用 LockGuard 自动加解锁，保证线程安全
        _task_queue.push(in);         // 将任务添加到任务队列尾部
        pthread_cond_signal(&_cond);  // 发送信号，唤醒一个等待在条件变量上的线程，通知有新任务
        {                             // 使用 CoutGuard 保护 std::cout
            CoutGuard coutGuard;
            std::cout << "向任务队列提交新任务: " << in.toString() << std::endl; // 输出任务提交信息
        }
    }

    // 停止线程池，优雅地关闭所有工作线程
    void stop()
    {
        {
            LockGuard lockguard(&_mutex); // 加锁，保证线程安全地修改 _is_shutdown 标志
            if (_is_shutdown)
                return;          // 如果已经停止，则直接返回
            _is_shutdown = true; // 设置线程池为停止状态
        }
        pthread_cond_broadcast(&_cond); // 广播条件变量，唤醒所有等待的线程，让他们检查停止状态
        {                               // 使用 CoutGuard 保护 std::cout
            CoutGuard coutGuard;
            std::cout << "ThreadPool 准备停止，通知所有线程..." << std::endl; // 输出线程池准备停止信息
        }
        for (const auto &t : _threads)
        {
            t->join(); // 等待所有线程结束
            delete t;  // 释放线程对象的内存
        }
        _threads.clear(); // 清空线程容器
        {                 // 使用 CoutGuard 保护 std::cout
            CoutGuard coutGuard;
            std::cout << "ThreadPool 已停止，所有线程已退出." << std::endl; // 输出线程池已停止信息
        }
    }

    // 析构函数，释放资源
    ~ThreadPool()
    {
        stop();                         // 在析构前先停止线程池，确保线程安全退出
        pthread_mutex_destroy(&_mutex); // 销毁互斥锁
        pthread_cond_destroy(&_cond);   // 销毁条件变量
        {                               // 使用 CoutGuard 保护 std::cout
            CoutGuard coutGuard;
            std::cout << "ThreadPool 资源已释放." << std::endl; // 输出线程池资源释放信息
        }
    }

    // 获取线程池单例实例的静态方法
    static ThreadPool<T> *getInstance()
    {
        // 双重检查锁模式，保证线程安全和效率
        if (nullptr == _instance)
        {
            std::lock_guard<std::mutex> lock(_single_mutex); // 使用 std::mutex 和 lock_guard 进行互斥，C++11 标准
            if (nullptr == _instance)
            {
                _instance = new ThreadPool<T>(); // 创建线程池实例
            }
        }
        return _instance; // 返回单例实例
    }

private:
    int _num;                       // 线程池中线程的数量
    std::vector<Thread *> _threads; // 线程池中的线程容器
    std::queue<T> _task_queue;      // 任务队列，存储待执行的任务
    pthread_mutex_t _mutex;         // 互斥锁，保护任务队列的线程安全访问
    pthread_cond_t _cond;           // 条件变量，用于线程等待新任务

    bool _is_shutdown; // 线程池是否停止的标志

    static ThreadPool<T> *_instance; // 线程池单例实例，静态成员变量
    static std::mutex _single_mutex; // 用于单例模式线程安全的互斥锁 (C++11 std::mutex)，静态成员变量
};

// 初始化静态成员变量
template <class T>
ThreadPool<T> *ThreadPool<T>::_instance = nullptr;

template <class T>
std::mutex ThreadPool<T>::_single_mutex;