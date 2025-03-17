// Thread.hpp: 定义了线程类 Thread，封装了 pthread 线程的创建、启动、等待和管理等操作。
//             提供了对 pthread 线程的 C++ 封装，简化了线程的创建和管理过程。
#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <cassert>
#include <functional>
#include <pthread.h>
#include <unistd.h>

namespace MyThread
{
    // 函数指针类型定义，线程回调函数类型
    typedef std::function<void *(void *)> func_t;
    const int num = 1024; // 缓冲区大小常量，例如线程名称缓冲区

    class Thread // 线程类，封装了 pthread 线程操作
    {
    private:
        // 静态线程函数，作为 pthread_create 的回调函数，是线程的入口点
        static void *thread_func(void *args) // 类内静态成员函数，作为线程回调函数
        {
            Thread *_this = static_cast<Thread *>(args); // 将 void* 参数转换为 Thread* 指针，获取 Thread 对象实例
            return _this->callback();                    // 调用 Thread 对象的 callback 方法，执行线程的具体任务
        }

    public:
        // 默认构造函数
        Thread()
        {
            char namebuffer[num];                                               // 线程名称缓冲区
            snprintf(namebuffer, sizeof(namebuffer), "thread-%d", threadnum++); // 生成线程名称，格式为 "thread-编号"
            _name = namebuffer;                                                 // 设置线程名称
        }

        // 启动线程，创建并运行线程
        void start(func_t func, void *args = nullptr)
        {
            _func = func;                                              // 设置线程需要执行的回调函数
            _args = args;                                              // 设置回调函数的参数，可以传递任意数据
            int n = pthread_create(&_tid, nullptr, thread_func, this); // 创建线程，_tid 存储线程ID，thread_func 是线程入口函数，this 指针作为参数传递给 thread_func
            assert(n == 0);                                            // 断言线程创建成功，如果创建失败，程序会终止
            (void)n;                                                   // 消除未使用变量警告，避免编译器报错
        }

        // 等待线程结束，阻塞调用线程直到目标线程执行完成
        void join()
        {
            int n = pthread_join(_tid, nullptr); // 等待线程 _tid 结束，阻塞当前线程直到 _tid 线程退出
            assert(n == 0);                      // 断言线程 join 成功，如果 join 失败，程序会终止
            (void)n;                             // 消除未使用变量警告
        }

        // 回调函数执行体，实际执行用户提供的回调函数
        void *callback()
        {
            return _func(_args); // 执行回调函数 _func，并将参数 _args 传递给它，返回回调函数的返回值
        }

        // 获取线程名称
        std::string threadname()
        {
            return _name; // 返回线程名称
        }

        // 析构函数
        ~Thread()
        {
            // do nothing，目前无需资源清理
        }

    private:
        std::string _name; // 线程名称，用于标识线程
        pthread_t _tid;    // 线程 ID，pthread 线程标识符
        func_t _func;      // 线程执行的回调函数，std::function 封装
        void *_args;       // 回调函数参数，void* 类型，可以传递任意类型数据

        static int threadnum; // 静态线程编号计数器，用于生成唯一线程名称
    };
    int Thread::threadnum = 1; // 初始化静态线程编号计数器，从 1 开始
}