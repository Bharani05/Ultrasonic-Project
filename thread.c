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
//thread_cancel

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>

void *thread_function(void *arg)
{
        char *s = (char *) arg;
        int count =0 ;
        printf("Thread: thread_function is started\n");
        while(1){
                sleep(1);
                printf("\nthread_function: executing loop\n");
        }
        sleep(5);

        printf("\nThread: thread_fuction exiting\n");
}
int main()
{
        pthread_t t_id;
//      void *res;
        int s, count;

        printf("\nMain thread is started:\n");
        s = pthread_create(&t_id, NULL, thread_function, NULL);
        if(s!=0)
                printf("\nProblem in thread creation \n");
        else
                printf("\nThread created Successfully:\n");
        for(count = 0; count < 5; count ++){
                sleep(2);
                printf("\nMain thread: count value = %d \n", count);
        }
        pthread_cancel(t_id);

        printf("\nMain thread exiting \n");
        pthread_exit(NULL);
}
//thread.c

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

void *thread_function(void *args)
{
        printf("Hello i am testing my thread concept\n");
        sleep(30);
}

int main()
{
        pthread_t t_id;
        int ret;

        printf("Process id = %d \n", getpid());
        printf("Before creation of thread\n");

        ret = pthread_create(&t_id, NULL, thread_function, NULL);
        if (ret == 0)
                printf("thread created successfully\n");
        else
                printf("Problem in thread creation\n");

        pthread_join(t_id, NULL);
        if(ret !=0)
                printf("pthread_join error\n");
        else
                printf("Thread terminated\n");
        return 0;
}
