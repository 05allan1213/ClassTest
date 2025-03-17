#include "name_pipe.hpp"

int main()
{
    bool ret = create_pipe(PIPE_NAME);
    assert(ret);
    (void)ret;

    std::cout << "server waiting client" << std::endl;
    int rfd = open(PIPE_NAME, O_RDONLY);
    std::cout << "server open pipe" << std::endl;
    if (rfd < 0)
        exit(1);

    // read from pipe
    char buf[1024];
    while (true)
    {
        ssize_t n = read(rfd, buf, sizeof(buf));
        if (n > 0)
        {
            buf[n] = 0;
            std::cout << "client->server: " << buf << std::endl;
        }
        else if (n == 0)
        {
            std::cout << "client closed" << std::endl;
            break;
        }
        else
        {
            std::cout << "read error" << std::endl;
            break;
        }
    }

    close(rfd);

    remove_pipe(PIPE_NAME);
    return 0;
}