//thread_app2.c
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>


void *thread_function(void *args);
void *thread_function_cancel(void *args);
void *thread_function_detach(void *args);
void *thread_function_mutex(void *args);
void *reader_function(void *args);
void *writer_function(void *args);
void *producer(void *args);
void *consumer(void *args);
void *spinlock_function(void *args);
void *wait_for_signal(void *args);
void *send_signal(void *args);


pthread_mutex_t lock;
pthread_rwlock_t rwlock;
sem_t empty, full;
pthread_spinlock_t spinlock;
pthread_cond_t cond;
int ready = 0;
int buffer[5];
int count = 0;

void menu() {
    printf("\nThread Operations Menu:\n");
    printf("1. Create Thread\n");
    printf("2. Cancel Thread\n");
    printf("3. Detach Thread\n");
    printf("4. Mutex Example\n");
    printf("5. Reader/Writer Example\n");
    printf("6. Semaphore Example\n");
    printf("7. Spinlock Example\n");
    printf("8. Condition Variable Example\n");
    printf("9. Start Sender Thread (for thread communication)\n");
    printf("10. Start Receiver Thread (for thread communication)\n");
    printf("11. Exit\n");
}

int main() {
    int choice;
    pthread_t thread;
    printf("Process id = %d \n", getpid());

    pthread_mutex_init(&lock, NULL);
    pthread_rwlock_init(&rwlock, NULL);
    sem_init(&empty, 0, 5);
    sem_init(&full, 0, 0);
    pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE);
    pthread_cond_init(&cond, NULL);

    while (1) {
        menu();
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                // Thread creation
                if (pthread_create(&thread, NULL, thread_function, NULL) == 0)
                    printf("Thread created successfully with ID: %lu\n", thread);
                else
                    printf("Thread creation failed\n");
                pthread_join(thread, NULL);
                break;

            case 2:
                // Thread cancellation
		
                if (pthread_create(&thread, NULL, thread_function_cancel, NULL) == 0) {
                  
                    sleep(1);
                    pthread_cancel(thread);
                    printf("Thread cancelled\n");
                } else {
                    printf("Thread creation failed\n");
                }
                pthread_join(thread, NULL);
                break;

            case 3:
                // Thread detachment
		//pthread_t thread_2;
                if (pthread_create(&thread, NULL, thread_function_detach, NULL) == 0)
                    printf("Thread created successfully...");
                else
                    printf("Thread creation failed\n");
                break;

            case 4:
		//mutex
                //pthread_t thread_3;
                for (long i = 0; i < 3; i++)
                    pthread_create(&thread, NULL, thread_function_mutex, (void *)i);
		//printf("Thread created successfully with ID: %lu\n", thread_3);
                pthread_join(thread, NULL);
                break;

            case 5:
                // Reader/Writer 
		//pthread_t thread_4;
		//pthread_t thread_5;
                for (long i = 0; i < 3; i++)
                    pthread_create(&thread, NULL, reader_function, (void *)i);
		//printf("Thread created successfully with ID: %lu\n", thread_4);
                for (long i = 0; i < 2; i++)
                    pthread_create(&thread, NULL, writer_function, (void *)(i + 3));
                pthread_join(thread, NULL);
                break;

            case 6:
                // Semaphore 
		//pthread_t thread_6;
		//pthread_t thread_7;
                pthread_create(&thread, NULL, producer, NULL);
		//printf("Thread created successfully with ID: %lu\n", thread);
                pthread_create(&thread, NULL, consumer, NULL);
		//printf("Thread created successfully with ID: %lu\n", thread);
                pthread_join(thread, NULL);
                break;

            case 7:
                // Spinlock 
		//pthread_t thread;
                for (long i = 0; i < 3; i++)
                    pthread_create(&thread, NULL, spinlock_function, (void *)i);
		//printf("Thread created successfully with ID: %lu\n", thread);
                pthread_join(thread, NULL);
                break;

            case 8:
                // Condition variable
		//pthread_t thread_9; 
		//pthread_t thread_10;
                pthread_create(&thread, NULL, wait_for_signal, (void *)0);
		//printf("Thread created successfully with ID: %lu\n", thread);
                pthread_create(&thread, NULL, send_signal, (void *)1);
		//printf("Thread created successfully with ID: %lu\n", thread);
                pthread_join(thread, NULL);
                break;

           /* case 9:
                // Start the sender thread
                pthread_create(&thread, NULL, send_signal, NULL);
		printf("Thread created successfully with ID: %lu\n", thread);
                pthread_join(thread, NULL);
                break;

            case 10:
                // Start the receiver thread
                pthread_create(&thread, NULL, wait_for_signal, NULL);
		printf("Thread created successfully with ID: %lu\n", thread);
                pthread_join(thread, NULL);
                break;*/

            case 11:
                // Exit the program
                printf("Exiting...\n");
                pthread_mutex_destroy(&lock);
                pthread_rwlock_destroy(&rwlock);
                sem_destroy(&empty);
                sem_destroy(&full);
                pthread_spin_destroy(&spinlock);
                pthread_cond_destroy(&cond);
                exit(0);

            default:
                printf("Invalid choice! Please try again.\n");
                break;
        }
    }

    return 0;
}

// Implementations of the functions for each case

void *thread_function(void *args) {
    printf("Thread function running\n");
    sleep(1);
    return NULL;
}

void *thread_function_cancel(void *args) {
    while (1) {
        printf("Thread running and waiting to be canceled...\n");
        sleep(1);
    }
    return NULL;
}

void *thread_function_detach(void *args) {
    pthread_detach(pthread_self());
    printf("Detached thread running\n");
    sleep(1);
    return NULL;
}

void *thread_function_mutex(void *args) {
    pthread_mutex_lock(&lock);
    printf("Thread %ld has entered the critical section.\n", (long)args);
    sleep(1);
    printf("Thread %ld is leaving the critical section.\n", (long)args);
    pthread_mutex_unlock(&lock);
    return NULL;
}

void *reader_function(void *args) {
    pthread_rwlock_rdlock(&rwlock);
    printf("Reader %ld is reading.\n", (long)args);
    sleep(1);
    printf("Reader %ld finished reading.\n", (long)args);
    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

void *writer_function(void *args) {
    pthread_rwlock_wrlock(&rwlock);
    printf("Writer %ld is writing.\n", (long)args);
    sleep(2);
    printf("Writer %ld finished writing.\n", (long)args);
    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

void *producer(void *args) {
    for (int i = 0; i < 4; i++) {
        sem_wait(&empty);
        pthread_mutex_lock(&lock);
        buffer[count++] = i;
        printf("Producer produced item %d\n", i);
        pthread_mutex_unlock(&lock);
        sem_post(&full);
        sleep(1);
    }
    return NULL;
}

void *consumer(void *args) {
    for (int i = 0; i < 4; i++) {
        sem_wait(&full);
        pthread_mutex_lock(&lock);
        int item = buffer[--count];
        printf("Consumer consumed item %d\n", item);
        pthread_mutex_unlock(&lock);
        sem_post(&empty);
        sleep(2);
    }
    return NULL;
}

void *spinlock_function(void *args) {
    pthread_spin_lock(&spinlock);
    printf("Thread %ld has entered the critical section.\n", (long)args);
    sleep(1);
    printf("Thread %ld is leaving the critical section.\n", (long)args);
    pthread_spin_unlock(&spinlock);
    return NULL;
}
void *send_signal(void *args) {
    sleep(2); // Delay to simulate time gap
    pthread_mutex_lock(&lock);
    ready = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);
    printf("Sender thread sent the signal.\n");
    return NULL;
}
void *wait_for_signal(void *args) {
    pthread_mutex_lock(&lock);
    while (!ready) {
        pthread_cond_wait(&cond, &lock);
    }
    printf("Receiver thread received the signal.\n");
    pthread_mutex_unlock(&lock);
    return NULL;


}

//thread_app3.c
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

// Function declarations
void *thread_function(void *args);
void *thread_function_cancel(void *args);
void *thread_function_detach(void *args);
void *thread_function_mutex(void *args);
void *reader_function(void *args);
void *writer_function(void *args);
void *producer(void *args);
void *consumer(void *args);
void *spinlock_function(void *args);
void *wait_for_signal(void *args);
void *send_signal(void *args);

// Global variables for different thread operations
pthread_mutex_t lock;
pthread_rwlock_t rwlock;
sem_t empty, full;
pthread_spinlock_t spinlock;
pthread_cond_t cond;
int ready = 0;
int buffer[5];
int count = 0;

void menu() {
    printf("\nThread Operations Menu:\n");
    printf("1. Create Thread\n");
    printf("2. Cancel Thread\n");
    printf("3. Detach Thread\n");
    printf("4. Mutex Example\n");
    printf("5. Reader/Writer Example\n");
    printf("6. Semaphore Example\n");
    printf("7. Spinlock Example\n");
    printf("8. Condition Variable Example\n");
    printf("9. Exit\n");
}

int main() {
    int choice;
    pthread_t thread;
    printf("Thread id = %d \n", getpid());

    pthread_mutex_init(&lock, NULL);
    pthread_rwlock_init(&rwlock, NULL);
    sem_init(&empty, 0, 5);
    sem_init(&full, 0, 0);
    pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE);
    pthread_cond_init(&cond, NULL);

    while (1) {
        menu();
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                // Thread creation example
                if (pthread_create(&thread, NULL, thread_function, NULL) == 0)
	
                    printf("Thread created successfully \n");
                else
                    printf("Thread creation failed\n");
                pthread_join(thread, NULL);
                break;

            case 2:
                // Thread cancellation example
                if (pthread_create(&thread, NULL, thread_function_cancel, NULL) == 0) {
                    printf("Thread created successfully\n");
                    sleep(1);
                    pthread_cancel(thread);
                    printf("Thread cancelled\n");
                } else {
                    printf("Thread creation failed\n");
                }
                pthread_join(thread, NULL);
                break;

            case 3:
                // Thread detachment example
                if (pthread_create(&thread, NULL, thread_function_detach, NULL) == 0)
                    printf("Thread created and detached successfully\n");
                else
                    printf("Thread creation failed\n");
                break;

            case 4:
                // Mutex example
                for (long i = 0; i < 3; i++)
                    pthread_create(&thread, NULL, thread_function_mutex, (void *)i);
                pthread_join(thread, NULL);
                break;

            case 5:
                // Reader/Writer example
                for (long i = 0; i < 3; i++)
                    pthread_create(&thread, NULL, reader_function, (void *)i);
                for (long i = 0; i < 2; i++)
                    pthread_create(&thread, NULL, writer_function, (void *)(i + 3));
                pthread_join(thread, NULL);
                break;

            case 6:
                // Semaphore example
                pthread_create(&thread, NULL, producer, NULL);
                pthread_create(&thread, NULL, consumer, NULL);
                pthread_join(thread, NULL);
                break;

            case 7:
                // Spinlock example
                for (long i = 0; i < 3; i++)
                    pthread_create(&thread, NULL, spinlock_function, (void *)i);
                pthread_join(thread, NULL);
                break;

            case 8:
                // Condition variable example
                pthread_create(&thread, NULL, wait_for_signal, (void *)0);
                pthread_create(&thread, NULL, send_signal, (void *)1);
                pthread_join(thread, NULL);
                break;

            case 9:
                // Exit the program
                printf("Exiting...\n");
                pthread_mutex_destroy(&lock);
                pthread_rwlock_destroy(&rwlock);
                sem_destroy(&empty);
                sem_destroy(&full);
                pthread_spin_destroy(&spinlock);
                pthread_cond_destroy(&cond);
                exit(0);

            default:
                printf("Invalid choice! Please try again.\n");
                break;
        }
    }

    return 0;
}

// Implementations of the functions for each case

void *thread_function(void *args) {
    printf("Thread function running\n");
    sleep(1);
    return NULL;
}

void *thread_function_cancel(void *args) {
    while (1) {
        printf("Thread running and waiting to be canceled...\n");
        sleep(1);
    }
    return NULL;
}

void *thread_function_detach(void *args) {
    pthread_detach(pthread_self());
    printf("Detached thread running\n");
    sleep(1);
    return NULL;
}

void *thread_function_mutex(void *args) {
    pthread_mutex_lock(&lock);
    printf("Thread %ld has entered the critical section.\n", (long)args);
    sleep(1);
    printf("Thread %ld is leaving the critical section.\n", (long)args);
    pthread_mutex_unlock(&lock);
    return NULL;
}

void *reader_function(void *args) {
    pthread_rwlock_rdlock(&rwlock);
    printf("Reader %ld is reading.\n", (long)args);
    sleep(1);
    printf("Reader %ld finished reading.\n", (long)args);
    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

void *writer_function(void *args) {
    pthread_rwlock_wrlock(&rwlock);
    printf("Writer %ld is writing.\n", (long)args);
    sleep(2);
    printf("Writer %ld finished writing.\n", (long)args);
    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

void *producer(void *args) {
    for (int i = 0; i < 4; i++) {
        sem_wait(&empty);
        pthread_mutex_lock(&lock);
        buffer[count++] = i;
        printf("Producer produced item %d\n", i);
        pthread_mutex_unlock(&lock);
        sem_post(&full);
        sleep(1);
    }
    return NULL;
}

void *consumer(void *args) {
    for (int i = 0; i < 4; i++) {
        sem_wait(&full);
        pthread_mutex_lock(&lock);
        int item = buffer[--count];
        printf("Consumer consumed item %d\n", item);
        pthread_mutex_unlock(&lock);
        sem_post(&empty);
        sleep(2);
    }
    return NULL;
}

void *spinlock_function(void *args) {
    pthread_spin_lock(&spinlock);
    printf("Thread %ld has entered the critical section.\n", (long)args);
    sleep(1);
    printf("Thread %ld is leaving the critical section.\n", (long)args);
    pthread_spin_unlock(&spinlock);
    return NULL;
}

void *wait_for_signal(void *args) {
    pthread_mutex_lock(&lock);
    while (!ready) {
        pthread_cond_wait(&cond, &lock);
    }
    printf("Thread %ld received the signal.\n", (long)args);
    pthread_mutex_unlock(&lock);
    return NULL;
}

void *send_signal(void *args) {
    sleep(2);
    pthread_mutex_lock(&lock);
    ready = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);
    printf("Thread %ld sent the signal.\n", (long)args);
    return NULL;
}

//thread_app.c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


pthread_t thread;
int thread_created = 0;
int thread_running = 1;  


void* threadFunction(void* arg) {
    printf("Thread %lu started, and waiting for instructions...\n", pthread_self());
    while (thread_running) {
        sleep(1); 
    }
    printf("Thread %lu is exiting...\n", pthread_self());
    pthread_exit(NULL);
}


void createThread() {
    if (thread_created) {
        printf("Thread already created. Cancel or exit it first.\n");
        return;
    }

    thread_running = 1;
    int rc = pthread_create(&thread, NULL, threadFunction, NULL);
    if (rc) {
        printf("Error: Unable to create thread.\n");
        exit(-1);
    }
    thread_created = 1;
    printf("Thread created successfully with ID: %lu\n", thread);
}


void cancelThread() {
    if (!thread_created) {
        printf("No thread to cancel.\n");
        return;
    }

    int rc = pthread_cancel(thread);
    if (rc) {
        printf("Error: Unable to cancel thread.\n");
    } else {
        printf("Thread %lu canceled successfully.\n", thread);
        thread_created = 0;
    }
}


void detachThread() {
    if (!thread_created) {
        printf("No thread to detach.\n");
        return;
    }

    int rc = pthread_detach(thread);
    if (rc) {
        printf("Error: Unable to detach thread.\n");
    } else {
        printf("Thread %lu detached successfully.\n", thread);
    }
}


void exitThread() {
    if (!thread_created) {
        printf("No thread to exit.\n");
        return;
    }

    thread_running = 0;  // Set flag to stop the thread
    pthread_join(thread, NULL);  // Wait for the thread to finish
    printf("Thread %lu exited successfully.\n", thread);
    thread_created = 0;
}

int main() {
    int choice;

    while (1) {
        printf("\nMenu:\n");
        printf("1. Create Thread\n");
        printf("2. Cancel Thread\n");
        printf("3. Detach Thread\n");
        printf("4. Exit Thread\n");
        printf("5. Quit Program\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                createThread();
                break;
            case 2:
                cancelThread();
                break;
            case 3:
                detachThread();
                break;
            case 4:
                exitThread();
                break;
            case 5:
                printf("Exiting program...\n");
                if (thread_created) {
                    cancelThread();
                }
                exit(0);
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }

    return 0;
}

//thread_gtk.c
#include <gtk/gtk.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Global variables for synchronization
pthread_mutex_t mutex;
pthread_rwlock_t rwlock;
sem_t empty, full;
pthread_spinlock_t spinlock;
pthread_cond_t cond;
int ready = 0;

// Thread functions
void *thread_function(void *arg) {
    printf("Hello from new thread\n");
    sleep(8);
    return NULL;
}

void *thread_function_cancel(void *arg) {
    printf("Thread function is running\n");
    while (1) {
        sleep(1);
        printf("Thread is running\n");
    }
    return NULL;
}

void *thread_function_detach(void *arg) {
    pthread_detach(pthread_self());
    printf("Detached thread function is running\n");
    sleep(5);
    printf("Detached thread function is exiting\n");
    return NULL;
}

void *thread_function_mutex(void *arg) {
    pthread_mutex_lock(&mutex);
    printf("Thread %ld has entered the critical section.\n", (long)arg);
    sleep(1);
    printf("Thread %ld is leaving the critical section.\n", (long)arg);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void *reader_function(void *arg) {
    pthread_rwlock_rdlock(&rwlock);
    printf("Reader %ld is reading.\n", (long)arg);
    sleep(1);
    printf("Reader %ld finished reading.\n", (long)arg);
    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

void *writer_function(void *arg) {
    pthread_rwlock_wrlock(&rwlock);
    printf("Writer %ld is writing.\n", (long)arg);
    sleep(2);
    printf("Writer %ld finished writing.\n", (long)arg);
    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

void *producer(void *arg) {
    int item;
    for (int i = 0; i < 4; i++) {
        item = i;
        sem_wait(&empty);
        pthread_mutex_lock(&mutex);

        printf("Producer produced item %d\n", item);

        pthread_mutex_unlock(&mutex);
        sem_post(&full);
        sleep(1);
    }
    return NULL;
}

void *consumer(void *arg) {
    int item;
    for (int i = 0; i < 4; i++) {
        sem_wait(&full);
        pthread_mutex_lock(&mutex);

        printf("Consumer consumed item %d\n", item);

        pthread_mutex_unlock(&mutex);
        sem_post(&empty);
        sleep(2);
    }
    return NULL;
}

void *spinlock_function(void *arg) {
    pthread_spin_lock(&spinlock);
    printf("Thread %ld has entered the critical section.\n", (long)arg);
    sleep(1);
    printf("Thread %ld is leaving the critical section.\n", (long)arg);
    pthread_spin_unlock(&spinlock);
    return NULL;
}

void *wait_for_signal(void *arg) {
    pthread_mutex_lock(&mutex);
    while (!ready) {
        pthread_cond_wait(&cond, &mutex);
    }
    printf("Thread %ld received the signal.\n", (long)arg);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void *send_signal(void *arg) {
    sleep(2);
    pthread_mutex_lock(&mutex);
    ready = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    printf("Thread %ld sent the signal.\n", (long)arg);
    return NULL;
}

// GTK callback functions
void on_create_thread_clicked(GtkWidget *widget, gpointer data) {
    pthread_t thread;
    pthread_create(&thread, NULL, thread_function, NULL);
    gtk_label_set_text(GTK_LABEL(data), "Thread created.");
}

void on_cancel_thread_clicked(GtkWidget *widget, gpointer data) {
    pthread_t thread;
    pthread_create(&thread, NULL, thread_function_cancel, NULL);
    sleep(1);
    pthread_cancel(thread);
    gtk_label_set_text(GTK_LABEL(data), "Thread canceled.");
}

void on_detach_thread_clicked(GtkWidget *widget, gpointer data) {
    pthread_t thread;
    pthread_create(&thread, NULL, thread_function_detach, NULL);
    gtk_label_set_text(GTK_LABEL(data), "Thread detached.");
}

void on_mutex_example_clicked(GtkWidget *widget, gpointer data) {
    pthread_t threads[3];
    pthread_mutex_init(&mutex, NULL);

    for (long i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, thread_function_mutex, (void*)i);
    }
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }
    pthread_mutex_destroy(&mutex);
    gtk_label_set_text(GTK_LABEL(data), "Mutex example completed.");
}

void on_rwlock_example_clicked(GtkWidget *widget, gpointer data) {
    pthread_t readers[3], writers[2];
    pthread_rwlock_init(&rwlock, NULL);

    for (long i = 0; i < 3; i++) {
        pthread_create(&readers[i], NULL, reader_function, (void*)i);
    }
    for (long i = 0; i < 2; i++) {
        pthread_create(&writers[i], NULL, writer_function, (void*)(i + 3));
    }
    for (int i = 0; i < 3; i++) {
        pthread_join(readers[i], NULL);
    }
    for (int i = 0; i < 2; i++) {
        pthread_join(writers[i], NULL);
    }
    pthread_rwlock_destroy(&rwlock);
    gtk_label_set_text(GTK_LABEL(data), "Read-Write Lock example completed.");
}

void on_semaphore_example_clicked(GtkWidget *widget, gpointer data) {
    pthread_t prod_thread, cons_thread;
    sem_init(&empty, 0, 5);
    sem_init(&full, 0, 0);
    pthread_mutex_init(&mutex, NULL);

    pthread_create(&prod_thread, NULL, producer, NULL);
    pthread_create(&cons_thread, NULL, consumer, NULL);
    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);

    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);
    gtk_label_set_text(GTK_LABEL(data), "Semaphore example completed.");
}

void on_spinlock_example_clicked(GtkWidget *widget, gpointer data) {
    pthread_t threads[3];
    pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE);

    for (long i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, spinlock_function, (void*)i);
    }
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_spin_destroy(&spinlock);
    gtk_label_set_text(GTK_LABEL(data), "Spinlock example completed.");
}

void on_cond_var_example_clicked(GtkWidget *widget, gpointer data) {
    pthread_t threads[2];
    pthread_cond_init(&cond, NULL);
    pthread_mutex_init(&mutex, NULL);

    pthread_create(&threads[0], NULL, wait_for_signal, (void*)0);
    pthread_create(&threads[1], NULL, send_signal, (void*)1);
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
    gtk_label_set_text(GTK_LABEL(data), "Condition Variable example completed.");
}

void on_exit_clicked(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *create_button, *cancel_button, *detach_button;
    GtkWidget *mutex_button, *rwlock_button, *semaphore_button;
    GtkWidget *spinlock_button, *condvar_button, *exit_button;
    GtkWidget *status_label;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Thread Control Application");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(on_exit_clicked), NULL);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), box);

    create_button = gtk_button_new_with_label("Create Thread");
    cancel_button = gtk_button_new_with_label("Cancel Thread");
    detach_button = gtk_button_new_with_label("Detach Thread");
    mutex_button = gtk_button_new_with_label("Mutex Example");
    rwlock_button = gtk_button_new_with_label("Read-Write Lock Example");
    semaphore_button = gtk_button_new_with_label("Semaphore Example");
    spinlock_button = gtk_button_new_with_label("Spinlock Example");
    condvar_button = gtk_button_new_with_label("Condition Variable Example");
    exit_button = gtk_button_new_with_label("Exit");

    gtk_box_pack_start(GTK_BOX(box), create_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), cancel_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), detach_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), mutex_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), rwlock_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), semaphore_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), spinlock_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), condvar_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), exit_button, TRUE, TRUE, 0);

    status_label = gtk_label_new("Status: ");
    gtk_box_pack_start(GTK_BOX(box), status_label, TRUE, TRUE, 0);

    g_signal_connect(create_button, "clicked", G_CALLBACK(on_create_thread_clicked), status_label);
    g_signal_connect(cancel_button, "clicked", G_CALLBACK(on_cancel_thread_clicked), status_label);
    g_signal_connect(detach_button, "clicked", G_CALLBACK(on_detach_thread_clicked), status_label);
    g_signal_connect(mutex_button, "clicked", G_CALLBACK(on_mutex_example_clicked), status_label);
    g_signal_connect(rwlock_button, "clicked", G_CALLBACK(on_rwlock_example_clicked), status_label);
    g_signal_connect(semaphore_button, "clicked", G_CALLBACK(on_semaphore_example_clicked), status_label);
    g_signal_connect(spinlock_button, "clicked", G_CALLBACK(on_spinlock_example_clicked), status_label);
    g_signal_connect(condvar_button, "clicked", G_CALLBACK(on_cond_var_example_clicked), status_label);
    g_signal_connect(exit_button, "clicked", G_CALLBACK(on_exit_clicked), NULL);

    sem_init(&empty, 0, 5);
    sem_init(&full, 0, 0);
    pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE);
    pthread_mutex_init(&mutex, NULL);
    pthread_rwlock_init(&rwlock, NULL);
    pthread_cond_init(&cond, NULL);

    gtk_widget_show_all(window);

    gtk_main();

    // Cleanup resources
    pthread_spin_destroy(&spinlock);
    pthread_mutex_destroy(&mutex);
    pthread_rwlock_destroy(&rwlock);
    pthread_cond_destroy(&cond);
    sem_destroy(&empty);
    sem_destroy(&full);

    return 0;
}

