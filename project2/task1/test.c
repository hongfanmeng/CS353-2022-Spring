#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void* pthread_func(void* arg)
{
    while (1);
}

int main(int argc, char* argv[])
{
    pthread_t tid[5];

    int cnt = 5;
    while (cnt--) {
        if (pthread_create(&tid[cnt], NULL, pthread_func, NULL) != 0) {
            fprintf(stderr, "pthread create fail\n");
            exit(-1);
        }
    }
    cnt = 5;
    while (cnt--) {
        pthread_join(tid[cnt], NULL);
    }
    return 0;
}