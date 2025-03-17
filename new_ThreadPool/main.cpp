//           项目目的：创建一个简单的线程池，能够处理用户输入的数学运算任务，并自动将任务执行信息和结果写入日志文件。
//           作用：展示如何使用 ThreadPool 类来并发执行任务，提高程序效率，同时提供任务日志记录功能，方便追踪和分析任务执行情况。
#include "Thread.hpp"
#include "ThreadPool.hpp"
#include "Task.hpp"
#include "LockGuard.hpp"
#include <unistd.h>
#include <string>

int main()
{
    // 获取线程池单例实例并运行
    ThreadPool<Task>::getInstance()->run();

    int x, y;            // 数学运算的操作数
    char op;             // 运算符
    std::string confirm; // 用于接收用户确认输入 (y/n)

    while (true) // 主循环，持续接收用户输入并提交任务
    {
        { // 使用 CoutGuard 保护 std::cout，线程安全输出
            CoutGuard coutGuard;
            std::cout << "请选择任务类型：1. 数学运算 (输入 1 或 'q' 退出) # "; // 简化提示信息，去除日志选项
        }
        std::cin >> confirm; // 读取用户选择的任务类型

        if (confirm == "q") // 如果用户输入 'q'，则退出程序
            break;

        if (confirm == "1") // 选择数学运算任务
        {
            { // 使用 CoutGuard 保护 std::cout
                CoutGuard coutGuard;
                std::cout << "请输入数据1# ";
            }
            std::cin >> x; // 读取第一个操作数
            {              // 使用 CoutGuard coutGuard;
                CoutGuard coutGuard;
                std::cout << "请输入数据2# ";
            }
            std::cin >> y; // 读取第二个操作数
            {              // 使用 CoutGuard coutGuard;
                CoutGuard coutGuard;
                std::cout << "请输入你要进行的运算(+ - * / %, 输入 'q' 退出) # ";
            }
            std::cin >> op; // 读取运算符

            Task t(x, y, op, mymath); // 创建数学运算任务 Task 对象
            {                         // 使用 CoutGuard coutGuard;
                CoutGuard coutGuard;
                std::cout << "您输入的数学任务是: " << t.toString() << "，确认提交吗? [y/n] "; // 询问用户是否提交数学任务
            }
            std::cin >> confirm; // 读取用户确认

            if (confirm == "y" || confirm == "Y") // 如果用户确认提交
            {
                ThreadPool<Task>::getInstance()->Push(t); // 向线程池提交数学任务
                {                                         // 使用 CoutGuard coutGuard;
                    CoutGuard coutGuard;
                    std::cout << "数学任务已提交到线程池." << std::endl; // 提示任务提交成功
                }
            }
            else // 用户取消提交
            {
                { // 使用 CoutGuard coutGuard;
                    CoutGuard coutGuard;
                    std::cout << "数学任务已取消." << std::endl; // 提示任务取消
                }
            }
        }
        else // 用户输入无效选项
        {
            { // Use CoutGuard to protect std::cout
                CoutGuard coutGuard;
                std::cout << "无效的选择，请重新输入或输入 'q' 退出." << std::endl; // 提示输入无效
            }
        }

        sleep(1); // 模拟主线程的其他工作，并控制任务提交速度
    }

    { // 使用 CoutGuard 保护 std::cout
        CoutGuard coutGuard;
        std::cout << "准备停止线程池..." << std::endl; // 提示准备停止线程池
    }
    ThreadPool<Task>::getInstance()->stop(); // 停止线程池，优雅关闭所有线程
    {                                        // 使用 CoutGuard coutGuard;
        CoutGuard coutGuard;
        std::cout << "线程池已停止，程序退出." << std::endl; // 提示线程池已停止，程序退出
    }

    return 0; // 主函数返回
}