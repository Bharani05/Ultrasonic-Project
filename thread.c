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
//mutex
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t mutex;
int shared_data = 0;

void* thread_function(void* arg) {
    pthread_mutex_lock(&mutex);
    // Critical section
    shared_data++;
    printf("Shared data incremented to: %d\n", shared_data);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    pthread_mutex_init(&mutex, NULL);

    pthread_create(&thread1, NULL, thread_function, NULL);
    pthread_create(&thread2, NULL, thread_function, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    pthread_mutex_destroy(&mutex);

    return 0;
}
//mutex_1
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 5

pthread_mutex_t mutex;
int shared_counter = 0;

void* increment_counter(void* arg) {
    for (int i = 0; i < 10000; ++i) {
        pthread_mutex_lock(&mutex);  // Lock the mutex
        shared_counter++;            // Critical section
        pthread_mutex_unlock(&mutex);// Unlock the mutex
    }
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];

    // Initialize the mutex
    pthread_mutex_init(&mutex, NULL);

    // Create threads
    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_create(&threads[i], NULL, increment_counter, NULL);
        printf("created thread id is = %d\n", getpid());
    }

    // Wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }

    // Destroy the mutex
    pthread_mutex_destroy(&mutex);

    printf("Final counter value: %d\n", shared_counter);

    return 0;
}
// sync
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t mutex;
pthread_cond_t cond_var;
int ready = 0;

void* producer(void* arg) {
    pthread_mutex_lock(&mutex);
    ready = 1;  // Set condition
    printf("Producer: Data is ready.\n");
    pthread_cond_signal(&cond_var);  // Signal the condition variable
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void* consumer(void* arg) {
    pthread_mutex_lock(&mutex);
    while (!ready) {
        pthread_cond_wait(&cond_var, &mutex);  // Wait for the condition var
iable
    }
    printf("Consumer: Consuming data.\n");
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main() {
    pthread_t prod_thread, cons_thread;

    // Initialize mutex and condition variable
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_var, NULL);

    // Create producer and consumer threads
    pthread_create(&prod_thread, NULL, producer, NULL);
    pthread_create(&cons_thread, NULL, consumer, NULL);

    // Wait for both threads to finish
    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);

    // Destroy mutex and condition variable
    pthread_cond_destroy(&cond_var);
    pthread_mutex_destroy(&mutex);

    return 0;
}
