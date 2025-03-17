//////////////////////////////////////////基于匿名管道的进程池设计//////////////////////////////////////////////////////////
/*
1. 创建多个子进程，并通过匿名管道（pipe(fd)）建立父子进程的通信信道。
2. 父进程负责任务分发，通过写管道向子进程发送任务编号。
3. 子进程循环读取任务编号，并执行相应的任务（downLoadTask()、upLoadTask() 等）。
4. 负载均衡调度：父进程随机选择子进程，并向其发送任务，实现简单的负载均衡。
5. 子进程退出：当父进程关闭所有写端，子进程会收到 read() 返回值 -1，然后退出。
6. 父进程等待子进程结束，调用 waitpid() 进行回收。
*/

#include <iostream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <functional>
#include <ctime>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MakeSeed() srand((unsigned long)time(nullptr) ^ getpid() ^ getppid() ^ rand() % 1213);
#define PROCESS_NUM 5

////////////////////////////////////////子进程要完成的某种任务///////////////////////////////////////////////////////////
typedef std::function<void()> func;

void downLoadTask()
{
    std::cout << getpid() << ": downloadTask" << std::endl;
    sleep(1);
}

void upLoadTask()
{
    std::cout << getpid() << ": uploadTask" << std::endl;
    sleep(1);
}

void ioTask()
{
    std::cout << getpid() << ": ioTask" << std::endl;
    sleep(1);
}

void flashTask()
{
    std::cout << getpid() << ": flashTask" << std::endl;
    sleep(1);
}

void loadTaskFunc(std::vector<func> *out)
{
    assert(out);
    out->push_back(downLoadTask);
    out->push_back(upLoadTask);
    out->push_back(ioTask);
    out->push_back(flashTask);
}
////////////////////////////////////////下面的代码是一个多进程程序//////////////////////////////////////////////////////
class subEndpoint
{
public:
    subEndpoint(pid_t subPid, int writeFd)
        : _subPid(subPid), _writeFd(writeFd)
    {
        char nameBuffer[1024];
        snprintf(nameBuffer, sizeof(nameBuffer), "process-%d[pid(%d)-fd(%d)]", num++, _subPid, _writeFd);
        _name = nameBuffer;
    }

    const std::string &getter_name() const
    {
        return _name;
    }

    pid_t getter_subPid() const
    {
        return _subPid;
    }

    int getter_writeFd() const
    {
        return _writeFd;
    }

private:
    static int num; // 静态变量，用于给每个子进程命名
    std::string _name;
    pid_t _subPid;
    int _writeFd;
};
int subEndpoint::num = 0;

int receveTask(int readFd)
{
    int code = 0;
    ssize_t n = read(readFd, &code, sizeof(code));
    if (n == 4)
        return code;
    else if (n <= 0)
        return -1;
    else
        return 0;
}

void sendTask(const subEndpoint &process, int taskNum)
{
    std::cout << "send task num: " << taskNum << " send to -> " << process.getter_name() << std::endl;
    int n = write(process.getter_writeFd(), &taskNum, sizeof(taskNum));
    assert(n == sizeof(int));
    (void)n;
}

void createSubProcess(std::vector<subEndpoint> *subs, std::vector<func> &funcMap)
{
    std::vector<int> deleteFd;
    for (int i = 0; i < PROCESS_NUM; i++)
    {
        int fd[2];
        int n = pipe(fd);
        assert(n == 0);
        (void)n;

        pid_t pid = fork();
        if (pid == 0)
        {
            for (int i = 0; i < deleteFd.size(); i++)
                close(deleteFd[i]);
            // 子进程，处理任务
            close(fd[1]); // 子进程关闭写端
            while (1)
            {
                // 1. 获取命令码，如果没有发送，子进程应该阻塞
                int commandCode = receveTask(fd[0]);
                // 2. 根据命令码，执行对应的任务
                if (commandCode >= 0 && commandCode < funcMap.size())
                    funcMap[commandCode]();
                else if (commandCode == -1)
                    break;
            }
            exit(0);
        }
        close(fd[0]); // 父进程关闭读端
        subEndpoint sub(pid, fd[1]);
        subs->push_back(std::move(sub));
        deleteFd.push_back(fd[1]);
    }
}

void makeRandSeed()
{
    srand((unsigned long)time(nullptr) ^ getpid() ^ getppid());
}

void loadBlanceContrl(const std::vector<subEndpoint> &subs, const std::vector<func> &funcMap, int count)
{
    int processnum = subs.size();
    int tasknum = funcMap.size();
    bool forever = (count == 0 ? true : false);
    while (true)
    {
        // 1. 选择一个子进程 --> std::vector<subEndpoint> subs[] --> 随机数
        int subIndex = rand() % processnum;
        // 2. 选择一个任务 --> std::vector<func> funcMap[]
        int taskIndex = rand() % tasknum;
        // 3. 向子进程发送任务
        sendTask(subs[subIndex], taskIndex);
        sleep(1);
        if (!forever)
        {
            count--;
            if (count == 0)
                break;
        }
    }
    for (int i = 0; i < processnum; i++)
    {
        close(subs[i].getter_writeFd());
    }
}

void waitProcess(const std::vector<subEndpoint> processes)
{
    int processnum = processes.size();
    for (int i = 0; i < processnum; i++)
    {
        waitpid(processes[i].getter_subPid(), nullptr, 0);
        std::cout << "wait sub process success ...: " << processes[i].getter_subPid() << std::endl;
    }
}
int main()
{
    MakeSeed();
    // 1.建立子进程并建立父子进程间通信
    // 1.1 加载方法表
    // (子进程pid,写入fd)
    std::vector<func> funcMap;
    loadTaskFunc(&funcMap);
    // 1.2 建立子进程，并维护好父子通信信道
    std::vector<subEndpoint> subs; // 引入数组方便下标访问子进程
    createSubProcess(&subs, funcMap);

    // 2.父进程，控制子进程，均衡地向子进程发送任务
    int taskNum = 10; // 如果为0，则一直执行
    loadBlanceContrl(subs, funcMap, taskNum);

    // 3.父进程，回收子进程
    waitProcess(subs);

    return 0;
}