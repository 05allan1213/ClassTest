#pragma once

#include <iostream>
#include <cstdio>
#include <string>
#include <functional>
#include <mutex>

std::mutex cerr_mutex;

class Task
{
    using func_t = std::function<int(int, int, char)>;

public:
    Task() = default;
    Task(int x, int y, char op, func_t func)
        : _x(x), _y(y), _op(op), _callback(func) {}

    std::string operator()()
    {
        int result = _callback(_x, _y, _op);

        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%d %c %d = %d", _x, _op, _y, result);
        return std::string(buffer);
    }

    std::string toString() const
    {
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%d %c %d = ?", _x, _op, _y);
        return std::string(buffer);
    }

private:
    int _x = 0;
    int _y = 0;
    char _op = '\0';
    func_t _callback;
};

const std::string oper = "+-*/%";

int mymath(int x, int y, char op)
{
    int result = 0;
    switch (op)
    {
    case '+':
        result = x + y;
        break;
    case '-':
        result = x - y;
        break;
    case '*':
        result = x * y;
        break;
    case '/':
        if (y == 0)
        {
            std::lock_guard<std::mutex> lock(cerr_mutex);
            std::cerr << "div zero error" << std::endl;
            result = -1;
        }
        else
        {
            result = x / y;
        }
        break;
    case '%':
        if (y == 0)
        {
            std::lock_guard<std::mutex> lock(cerr_mutex);
            std::cerr << "mod zero error" << std::endl;
            result = -1;
        }
        else
        {
            result = x % y;
        }
        break;
    default:
        break;
    }
    return result;
}

class Save
{
    using func_t = std::function<void(const std::string &)>;

public:
    Save() = default;
    Save(std::string message, func_t func)
        : _message(std::move(message)), _callback(std::move(func)) {}

    void operator()() { _callback(_message); }

private:
    std::string _message;
    func_t _callback;
};

void save(const std::string &message)
{
    const std::string path = "./log.txt";
    FILE *fp = fopen(path.c_str(), "a+");
    if (!fp)
    {
        std::cerr << "open file error" << std::endl;
        return;
    }
    fputs(message.c_str(), fp);
    fputc('\n', fp);
    fflush(fp); // 确保数据写入
    fclose(fp);
}
