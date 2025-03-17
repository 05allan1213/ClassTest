#ifndef _SHM_HPP_
#define _SHM_HPP_

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define PATHNAME "/tmp/shm_test"
#define PROJ_ID 0x66
#define MAX_SIZE 4096

key_t get_key(const char *pathname, int proj_id)
{
    key_t key;
    if ((key = ftok(pathname, proj_id)) == -1)
    {
        std::cerr << errno << ":" << strerror(errno) << std::endl;
        exit(1);
    }
    return key;
}

int getShmHelper(key_t key, int flag)
{
    int shmid = shmget(key, MAX_SIZE, flag);
    if (shmid < 0)
    {
        std::cerr << errno << ":" << strerror(errno) << std::endl;
        exit(2);
    }
    return shmid;
}

int create_shm(key_t key)
{
    return getShmHelper(key, IPC_CREAT | IPC_EXCL | 0666);
}

int get_shm(key_t key)
{
    return getShmHelper(key, IPC_CREAT | 0666);
}

void *attach_shm(int shmid)
{
    void *shmaddr = shmat(shmid, NULL, 0);
    if (shmaddr == (void *)-1)
    {
        std::cerr << errno << ":" << strerror(errno) << std::endl;
        exit(3);
    }
    return shmaddr;
}

void detach_shm(void *shmaddr)
{
    if (shmdt(shmaddr) < 0)
    {
        std::cerr << errno << ":" << strerror(errno) << std::endl;
        exit(4);
    }
}
void delete_shm(int shmid)
{
    if (shmctl(shmid, IPC_RMID, NULL) < 0)
    {
        std::cerr << errno << ":" << strerror(errno) << std::endl;
        exit(5);
    }
}

#endif