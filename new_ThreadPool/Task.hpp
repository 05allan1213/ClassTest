// Task.hpp: 定义了任务类 Task，用于封装需要线程池执行的任务。

#pragma once

#include <iostream>
#include <cstdio>
#include <string>
#include <functional>
#include <mutex>
#include "LockGuard.hpp"

std::mutex cerr_mutex; // 用于保护 std::cerr 输出的互斥锁，避免多线程同时输出到错误流导致信息错乱

// Task 类可以表示不同类型的任务，当前示例中主要用于数学运算任务。
class Task
{
    using func_t = std::function<int(int, int, char)>; // 定义函数类型别名 func_t，代表一个接受两个 int 和一个 char 参数，返回 int 的函数

public:
    Task() = default; // 默认构造函数
    // 带参数构造函数，用于数学运算任务
    Task(int x, int y, char op, func_t func)
        : _x(x), _y(y), _op(op), _callback(func) {}

    // 函数对象 operator() 重载，使得 Task 对象可以像函数一样被调用，执行任务并返回结果字符串
    std::string operator()()
    {
        int result = _callback(_x, _y, _op); // 调用回调函数 _callback (mymath) 执行实际运算，获取运算结果

        char buffer[256];                                                       // 增加 buffer 大小以容纳更长的日志消息，用于格式化结果字符串
        snprintf(buffer, sizeof(buffer), "%d %c %d = %d", _x, _op, _y, result); // 格式化为 "x op y = result" 形式
        return std::string(buffer);                                             // 返回结果字符串，包含数学运算结果
    }

    // 转换为字符串，用于日志记录或输出任务信息，不执行任务本身，仅返回任务描述
    std::string toString() const
    {
        char buffer[64];                                               // 缓冲区，用于格式化任务描述字符串
        snprintf(buffer, sizeof(buffer), "%d %c %d = ?", _x, _op, _y); // 格式化为 "x op y = ?" 形式，不包含结果
        return std::string(buffer);                                    // 返回任务描述字符串
    }

private:
    int _x = 0;       // 操作数 1，数学运算任务的输入
    int _y = 0;       // 操作数 2，数学运算任务的输入
    char _op = '\0';  // 运算符，标识任务类型，例如 '+', '-', '*', '/', '%'
    func_t _callback; // 回调函数，指向实际执行运算的函数 (如 mymath)
};

const std::string oper = "+-*/%"; // 运算符字符串，未使用

// 实际的运算和日志处理函数，根据运算符执行不同的操作
int mymath(int x, int y, char op)
{
    int result = 0; // 存储运算结果
    switch (op)     // 根据运算符 op 进行分支处理
    {
    case '+': // 加法
        result = x + y;
        break;
    case '-': // 减法
        result = x - y;
        break;
    case '*': // 乘法
        result = x * y;
        break;
    case '/':       // 除法
        if (y == 0) // 检查除数是否为零
        {
            { // 使用 CoutGuard 保护 std::cerr，线程安全错误输出
                CoutGuard coutGuard;
                std::cerr << "Error: Division by zero! 任务无法完成." << std::endl; // 输出除零错误信息
            }
            result = -1; // 返回错误码 -1 表示除零错误
        }
        else
        {
            result = x / y; // 执行除法运算
        }
        break;
    case '%':       // 取模
        if (y == 0) // 检查模数是否为零
        {
            { // 使用 CoutGuard 保护 std::cerr，线程安全错误输出
                CoutGuard coutGuard;
                std::cerr << "Error: Modulo by zero! 任务无法完成." << std::endl; // 输出模零错误信息
            }
            result = -1; // 返回错误码 -1 表示模零错误
        }
        else
        {
            result = x % y; // 执行取模运算
        }
        break;
    default: // 默认情况，运算符不在支持列表中
        break;
    }
    return result; // 返回运算结果或日志任务状态
}

// 实际的保存函数，将日志消息保存到文件
void save(const std::string &message)
{
    const std::string path = "./log.txt"; // 日志文件路径
    FILE *fp = fopen(path.c_str(), "a+"); // 以追加模式打开文件，如果文件不存在则创建
    if (!fp)                              // 检查文件是否成功打开
    {
        { // 使用 CoutGuard 保护 std::cerr，线程安全错误输出
            CoutGuard coutGuard;
            std::cerr << "open file error" << std::endl; // 输出文件打开失败错误信息
        }
        return; // 文件打开失败，直接返回
    }
    fputs(message.c_str(), fp); // 将日志消息写入文件
    fputc('\n', fp);            // 写入换行符，使每条日志消息占一行
    fflush(fp);                 // 刷新文件缓冲区，确保数据立即写入磁盘
    fclose(fp);                 // 关闭文件，释放文件资源
}

// Save 类，用于演示不同类型的任务，例如保存日志（当前未使用在 main.txt 中，日志功能直接在 Task 中实现）
class Save
{
    using func_t = std::function<void(const std::string &)>; // 定义函数类型别名 func_t，代表一个接受 std::string 参数，返回 void 的函数

public:
    Save() = default; // 默认构造函数
    Save(std::string message, func_t func)
        : _message(std::move(message)), _callback(std::move(func)) {} // 构造函数，接收日志消息和回调函数，使用 move 语义避免不必要的拷贝

    // 函数对象 operator() 重载，执行保存操作
    void operator()() { _callback(_message); } // 调用回调函数 _callback (save) 执行保存操作，传递日志消息 _message

private:
    std::string _message; // 要保存的日志消息
    func_t _callback;     // 回调函数，指向实际的保存函数 (save)
};