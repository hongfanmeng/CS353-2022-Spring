#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void* pthread_func(void* arg)
{
    while (1);
}

static void set_pthread_attr(pthread_attr_t* attr)
{
    struct sched_param param;
    pthread_attr_init(attr);
    pthread_attr_setschedpolicy(attr, SCHED_FIFO);
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    pthread_attr_setschedparam(attr, &param);
    pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED);

}

int main(int argc, char* argv[])
{
    pthread_t tid;
    pthread_attr_t attr;

    set_pthread_attr(&attr);

    if (pthread_create(&tid, &attr, pthread_func, NULL) != 0) {
        fprintf(stderr, "pthread create fail\n");
        exit(-1);
    }
    pthread_join(tid, NULL);
    return 0;
}