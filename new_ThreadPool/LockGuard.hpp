// LockGuard.hpp: 定义了 Mutex 类和 LockGuard 类，用于实现基于 RAII 风格的互斥锁管理。
#pragma once

#include <pthread.h>
#include <mutex>

// Mutex 类，封装 pthread 互斥锁和 std::mutex，提供统一的 Lock 和 Unlock 接口
class Mutex
{
public:
    // 构造函数，接收 pthread 互斥锁指针
    Mutex(pthread_mutex_t *lock) : _lock(lock)
    {
    }
    // 构造函数，接收 std::mutex 指针
    Mutex(std::mutex *lock) : _std_lock(lock), _lock(nullptr) {}

    // 加锁操作，根据内部存储的互斥锁类型调用相应的加锁函数
    void Lock()
    {
        if (_lock) // 如果是 pthread 互斥锁
            pthread_mutex_lock(_lock);
        else if (_std_lock) // 如果是 std::mutex
            _std_lock->lock();
    }
    // 解锁操作，根据内部存储的互斥锁类型调用相应的解锁函数
    void Unlock()
    {
        if (_lock) // 如果是 pthread 互斥锁
            pthread_mutex_unlock(_lock);
        else if (_std_lock) // 如果是 std::mutex
            _std_lock->unlock();
    }
    // 析构函数，目前为空，如有资源清理可在此添加
    ~Mutex()
    {
    }

    pthread_mutex_t *_lock = nullptr; // 指向 pthread 互斥锁的指针，默认为空
    std::mutex *_std_lock = nullptr;  // 指向 std::mutex 的指针，默认为空
};

// LockGuard 类，RAII 风格的互斥锁Guard，利用作用域自动管理互斥锁的生命周期
class LockGuard
{
public:
    // 构造函数，接收 pthread 互斥锁指针，构造时自动加锁
    LockGuard(pthread_mutex_t *lock) : _mutex(lock)
    {
        _mutex.Lock(); // 构造时加锁，通过调用 Mutex 类的 Lock 方法
    }

    // 构造函数，接收 std::mutex 指针，构造时自动加锁
    LockGuard(std::mutex *lock) : _mutex(lock)
    {
        _mutex.Lock(); // 构造时加锁，通过调用 Mutex 类的 Lock 方法
    }

    // 析构函数，作用域结束时自动解锁
    ~LockGuard()
    {
        _mutex.Unlock(); // 析构时解锁，通过调用 Mutex 类的 Unlock 方法，确保互斥锁在作用域结束时被释放
    }

private:
    Mutex _mutex; // 内部持有的 Mutex 对象，用于管理互斥锁
};

// 用于保护 std::cout 的互斥锁，静态全局变量
static std::mutex cout_mutex;

// CoutGuard 类，RAII 风格的 cout 互斥锁Guard，用于保护 std::cout 的线程安全输出
class CoutGuard
{
public:
    // 构造函数，构造 CoutGuard 对象时自动对 cout_mutex 加锁
    CoutGuard() : _coutMutex(&cout_mutex)
    {
        _coutMutex.Lock(); // 构造时加锁，保证在 CoutGuard 对象存在期间 std::cout 的访问是互斥的
    }
    // 析构函数，CoutGuard 对象销毁时自动解锁 cout_mutex
    ~CoutGuard()
    {
        _coutMutex.Unlock(); // 析构时解锁，释放 std::cout 的互斥访问权
    }

private:
    Mutex _coutMutex; // 内部持有的 Mutex 对象，用于管理 cout_mutex
};