#include "name_pipe.hpp"

int main()
{
    int wfd = open(PIPE_NAME, O_WRONLY);
    std::cout << "client open pipe" << std::endl;
    if (wfd < 0)
        exit(1);

    // write to pipe
    char buf[1024];
    while (true)
    {
        std::cout << "Enter message: ";
        fgets(buf, sizeof(buf), stdin);
        if (strlen(buf) > 0)
            buf[strlen(buf) - 1] = 0;
        ssize_t n = write(wfd, buf, strlen(buf));
        assert(n == strlen(buf));
        (void)n;
    }
    close(wfd);
    return 0;
}