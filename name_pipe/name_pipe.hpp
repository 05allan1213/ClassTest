#pragma once

#include <iostream>
#include <cstring>
#include <cassert>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PIPE_NAME "/tmp/name_pipe"

bool create_pipe(const std::string &path)
{
    unlink(path.c_str());
    int ret = mkfifo(path.c_str(), 0666);
    if (ret == 0)
        return true;
    else
    {
        std::cout << "mkfifo error" << std::endl;
        return false;
    }
}

void remove_pipe(const std::string &path)
{
    int ret = unlink(path.c_str());
    assert(ret == 0);
    (void)ret;
}