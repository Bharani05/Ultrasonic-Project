#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

void *thread_function(void *arg)
{
        char *s = (char *) arg;
        //int count = 0;
        printf("thread_function is started \n");

        pthread_detach(pthread_self());
        //printf("pthread_self() = %d \n",pthread_self());
        sleep(5);
        printf("thread_function is exiting now \n");
}
int main()
{
        pthread_t t1_id;
        void *res;
        int s;

        s = pthread_create(&t1_id, NULL, thread_function, NULL);
        printf("Thread id is = %d \n", getpid());
        if(s!=0)
                printf("problem in thread creation:\n");
        else
                printf("thread created successfully:\n");
        sleep(3);
        printf("main thread is exiting now:\n");
        pthread_exit(NULL);
}
