#include "shm.hpp"

int main()
{
    key_t key = get_key(PATHNAME, PROJ_ID);
    printf("key: 0x%x\n", key);
    int shmid = get_shm(key);
    printf("shmid: %d\n", shmid);

    char *shm = (char *)attach_shm(shmid);
    printf("attach success, address: %p\n", shm);

    const char *message = "Hello server,I'm client";
    pid_t pid = getpid();
    int count = 1;
    while (true)
    {
        sleep(1);
        // pid,count,message
        snprintf(shm, MAX_SIZE, "%s[pid:%d][msg_id:%d]", message, pid, count++);
    }

    detach_shm(shm);

    return 0;
}