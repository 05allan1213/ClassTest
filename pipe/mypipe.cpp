#include <iostream>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    // 第一步：创建管道，打开读写端
    int fd[2];
    int n = pipe(fd);
    assert(n == 0);

    // 第二步：创建子进程
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0)
    {
        // 子进程进行写入
        close(fd[0]);
        // 子进程的通信端
        const char *msg = "I am child,I'm chatting with my parent";
        int counut = 0;
        while (true)
        {
            counut++;
            char buf[1024];
            snprintf(buf, sizeof(buf), "child->parent: %s[%d][%d]", msg, counut, getpid());
            write(fd[1], buf, strlen(buf));
            sleep(1); // 模拟子进程的休眠
        }
        // 子进程
        close(fd[1]);
        exit(0);
    }
    // 父进程进行读取
    close(fd[1]);
    // 父进程的通信端
    while (true)
    {
        char buf[1024];
        ssize_t sz = read(fd[0], buf, sizeof(buf) - 1); // 将数据当成字符串处理
        if (sz > 0)
        {
            buf[sz] = 0;
            std::cout << "Get message from child: " << buf << "| my pid: " << getpid() << std::endl;
        }
        // 细节：父进程没有进行sleep
    }

    n = waitpid(pid, nullptr, 0);
    assert(n == pid);
    close(fd[0]);

    // fd[0] : read
    // fd[1] : write
    // std::cout << "fd[0] = " << fd[0] << std::endl;
    // std::cout << "fd[1] = " << fd[1] << std::endl;
    return 0;
}