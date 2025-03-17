#include "shm.hpp"

int main()
{
    key_t key = get_key(PATHNAME, PROJ_ID);
    printf("key: 0x%x\n", key);

    int old_shmid = get_shm(key);
    if (old_shmid != -1)
    {
        printf("old shmid: %d\n", old_shmid);
        delete_shm(old_shmid);
        printf("delete old shm success\n");
    }

    int shmid = create_shm(key);
    printf("shmid: %d\n", shmid);

    sleep(5);

    // attach
    char *shm = (char *)attach_shm(shmid);
    printf("attach success, address: %p\n", shm);

    // use
    while (true)
    {
        printf("client say : %s\n", shm);
        sleep(1);
    }

    // detach
    detach_shm(shm);

    sleep(10);

    // delete
    delete_shm(shmid);

    return 0;
}