#include <iostream>
#include <cassert>
#include <unistd.h>
#include <signal.h>
using namespace std;

static void handler(int signo)
{
    cout << signo << " 号信号确实递达了" << endl;
    // 最终不退出进程
}

static void DisplayPending(const sigset_t pending)
{
    // 打印 pending 表
    cout << "当前进程的 pending 表为: ";
    int i = 1;
    while (i < 32)
    {
        if (sigismember(&pending, i))
            cout << "1";
        else
            cout << "0";

        i++;
    }
    cout << endl;
}

int main()
{
    // 更改 2 号信号的执行动作
    signal(2, handler);

    // 创建信号集
    sigset_t set, oset;

    // 初始化信号集
    sigemptyset(&set);
    sigemptyset(&oset);

    // 阻塞2号信号
    sigaddset(&set, 2); // 记录 2 号信号

    // 设置当前进程的 屏蔽信号集
    sigprocmask(SIG_BLOCK, &set, &oset);

    // 死循环
    int n = 0;
    while (true)
    {
        if (n == 5)
        {
            // 采用 SIG_SETMASK 的方式，覆盖进程的 block 表
            sigprocmask(SIG_SETMASK, &oset, nullptr); // 不接收进程的 block 表
        }

        // 获取进程的 未决信号集
        sigset_t pending;
        sigemptyset(&pending);

        int ret = sigpending(&pending);
        assert(ret == 0);
        (void)ret; // 欺骗编译器，避免 release 模式中出错

        DisplayPending(pending);

        n++;
        sleep(1);
    }

    return 0;
}
