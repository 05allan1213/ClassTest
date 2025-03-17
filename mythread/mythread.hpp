#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <cassert>
#include <functional>
#include <pthread.h>
#include <unistd.h>

class Thread;

class Context
{
public:
    Thread *_this;
    void *_args;

public:
    Context()
        : _this(nullptr), _args(nullptr)
    {
    }
    ~Context()
    {
    }
};

class Thread
{
public:
    typedef std::function<void *(void *)> func_t;
    const int num = 1024;

public:
    Thread(func_t func, void *args, int number = 0)
        : _func(func), _args(args)
    {
        char buffer[num];
        snprintf(buffer, sizeof(buffer), "thread-%d", number);
        _name = buffer;
    }

    // 在类内部创建线程，若希望线程执行对应的方法，需要将方法设为 static
    static void *thread_func(void *args) // 类内成员，有隐藏参数
    {
        Context *ctx = static_cast<Context *>(args);
        ctx->_this->run(ctx->_args);

        delete ctx;
        // 非静态方法不能调用成员方法或成员变量
        // return _func(_args);
    }

    void start()
    {
        Context *ctx = new Context();
        ctx->_this = this;
        ctx->_args = _args;

        int n = pthread_create(&_tid, nullptr, thread_func, ctx);
        assert(n == 0);
        (void)n;
    }

    void join()
    {
        int n = pthread_join(_tid, nullptr);
        assert(n == 0);
        (void)n;
    }

    void *run(void *args)
    {
        return _func(args);
    }

    ~Thread()
    {
        // do nothing
    }

private:
    std::string _name;
    pthread_t _tid;
    func_t _func;
    void *_args;
};
