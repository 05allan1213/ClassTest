#include "Thread.hpp"
#include "ThreadPool.hpp"
#include "Task.hpp"
int main()
{
    // std::unique_ptr<ThreadPool<Task>> pool(new ThreadPool<Task>());
    // pool->run();
    ThreadPool<Task>::getInstance()->run();

    int x, y;
    char op;

    while (true)
    {
        std::cout << "请输入数据1# ";
        std::cin >> x;
        std::cout << "请输入数据2# ";
        std::cin >> y;
        std::cout << "请输入你要进行的运算(+ - * / %) # ";
        std::cin >> op;

        Task t(x, y, op, mymath);
        ThreadPool<Task>::getInstance()->Push(t);
        // std::cout << "刚刚录入一个任务：" << t.toString() << ", 确认提交吗? [y/n] ";
        // std::cin.get();
        // if (std::cin.get() == 'y')
        // {
        //     pool->Push(t);
        //     std::cout << "提交成功" << std::endl;
        // }

        sleep(1);
    }
    return 0;
}
