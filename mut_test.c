// mutex_test.c -- X/OS mutex test

// Copyright (c) 2003-2014 Cadence Design Systems, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


#include "test.h"


// Mutex tests:
// (1) Create 3 threads that compete for a single mutex. Verify that
// all threads complete successfully. Mutex wait policy is by priority.
// (2) Repeat test with mutex wait policy = fifo.


#define NUM_THREADS  3
#define REP_COUNT    3000


#if !USE_MAIN_THREAD
// If main() is not converted into a thread, then allocate space
// for the test main thread.
XosThread mutex_test_tcb;
uint8_t   mutex_test_stk[STACK_SIZE];
#endif

XosThread test_threads[NUM_THREADS];
uint8_t   test_stacks[NUM_THREADS][STACK_SIZE];
char *    t_names[] =
{
    "Thread_1",
    "Thread_2",
    "Thread_3",
};

static XosMutex         test_mutex_1;
static volatile int32_t flag = 0;

#if XCHAL_NUM_TIMERS > 0
static int32_t          have_timer = 1;
#else
static int32_t          have_timer = 0;
#endif


//-----------------------------------------------------------------------------
// Basic checks. This function is called from both normal thread context
// as well as interrupt context.
//-----------------------------------------------------------------------------
static int32_t
mutex_basic_checks(int32_t from_int)
{
    XosMutex mtx1;
    int32_t  ret;

    // Check create
    ret = xos_mutex_create(&mtx1, 0, 0);
    if (ret != XOS_OK)
        return ret;

    // Check lock
    if (!from_int) {
        ret = xos_mutex_lock(&mtx1);
        if (ret != XOS_OK)
            return ret;
        ret = xos_mutex_unlock(&mtx1);
        if (ret != XOS_OK)
            return ret;
    }

    // Check recursive locking
    if (!from_int) {
        int32_t i;

        ret = xos_mutex_lock(&mtx1);
        if (ret != XOS_OK)
            return ret;

        for (i = 0; i < 5; i++) {
            ret = xos_mutex_lock(&mtx1);
            if (ret != XOS_OK)
                return ret;
        }

        for (i = 0; i < 6; i++) {
            ret = xos_mutex_unlock(&mtx1);
            if (ret != XOS_OK)
                return ret;
        }

        // Verify that mutex is now unlocked
        ret = xos_mutex_test(&mtx1);
        if (ret != 0)
            return ret;
    }

    // Trylock
    ret = xos_mutex_trylock(&mtx1);
    if (ret != XOS_OK)
        return ret;

    // Unlock
    ret = xos_mutex_unlock(&mtx1);
    if (ret != XOS_OK)
        return ret;

    // Destroy
    ret = xos_mutex_delete(&mtx1);
    return ret;
}


//-----------------------------------------------------------------------------
// Timer callback for basic tests.
//-----------------------------------------------------------------------------
void
mutex_timer_cb(void * arg)
{
    int32_t * p = (int32_t *) arg;

    *p = mutex_basic_checks(1);
    flag = 1;
}


//-----------------------------------------------------------------------------
// Thread function. Waits on mutex and increments a counter every time
// it gets ownership until target count is reached.
//-----------------------------------------------------------------------------
static int32_t
mutex_get_func(void * arg, int32_t unused)
{
    uint32_t * pval = (uint32_t *) arg; // Result storage
    uint32_t   cnt  = 0;
    char       buf[64];

    //printf("+ Thread %s starting\n", xos_thread_get_name(xos_thread_id()));

    while (cnt < REP_COUNT) {
        int32_t ret = xos_mutex_lock(&test_mutex_1);

        if (ret != XOS_OK) {
            printf("ERROR: xos_mutex_lock() ret %d\n", ret);
            return cnt;
        }

        cnt++;

        // Now do something for long enough to encounter a context switch.
        // sprintf() is conveniently long.
        sprintf(buf, "Thread %s got mutex cnt = %u\n", xos_thread_get_name(xos_thread_id()), cnt);
        *pval = cnt;
        xos_mutex_unlock(&test_mutex_1);
   }

    return cnt;
}


//-----------------------------------------------------------------------------
// Mutex multi-thread test.
//-----------------------------------------------------------------------------
static int32_t
mutex_thread_test(int32_t flag)
{
    uint32_t cnt[NUM_THREADS];
    uint32_t total = 0;
    int32_t  val;
    int32_t  ret;
    int32_t  i;
    
    if (flag) {
        ret = xos_mutex_create(&test_mutex_1, XOS_MUTEX_WAIT_FIFO, 0);
    }
    else {
        ret = xos_mutex_create(&test_mutex_1, XOS_MUTEX_WAIT_PRIORITY, 0);
    }
    
    if (ret != XOS_OK)
        return ret;

    for (i = 0; i < NUM_THREADS; i++) {
        cnt[i] = 0;
        ret = xos_thread_create(&test_threads[i],
                                0,
                                mutex_get_func,
                                &cnt[i],
                                t_names[i],
                                test_stacks[i],
                                STACK_SIZE,
                                flag ? 1 : (i+1),
                                0,
                                0);
        if (ret) {
            printf("ERROR: creating test threads\n");
            return ret;
        }
    }

    for (i = 0; i < NUM_THREADS; i++) {
        ret = xos_thread_join(&test_threads[i], &val);
        if (ret) {
            printf("ERROR: joining thread\n");
            return ret;
        }
        if (val != REP_COUNT) {
            printf("ERROR: thread count mismatch\n");
            return 1;
        }
        total += val;
    }

    if (total != NUM_THREADS * REP_COUNT) {
        printf("ERROR: total count mismatch\n");
        return 1;
    }

    xos_list_threads();

    for (i = 0; i < NUM_THREADS; i++) {
        xos_thread_delete(&test_threads[i]);
    }
    xos_mutex_delete(&test_mutex_1);

    return 0;
}


//-----------------------------------------------------------------------------
// Thread function to check mutex timeout operation.
//-----------------------------------------------------------------------------
static int32_t 
mutex_timeout_func(void * arg, int32_t unused)
{
    int32_t    ret;
    XosMutex * mtx = (XosMutex *) arg;

    // This get should fail, because the parent thread already has the
    // mutex locked. So we should get a timeout return.
    ret = xos_mutex_lock_timeout(mtx, xos_secs_to_cycles(1));

    if (ret == XOS_ERR_TIMEOUT)
        return XOS_OK;

    return ret;
}


//-----------------------------------------------------------------------------
// Timeout test.
//-----------------------------------------------------------------------------
static int32_t
mutex_timeout_test(void)
{
    XosMutex mtx2;
    int32_t  ret;
    int32_t  val;

    ret = xos_mutex_create(&mtx2, 0, 0);
    if (ret != XOS_OK)
        return ret;

    // Lock the mutex
    ret = xos_mutex_lock(&mtx2);
    if (ret != XOS_OK)
        return ret;

    // Create the test thread
    ret = xos_thread_create(&test_threads[0],
                            0,
                            mutex_timeout_func,
                            &mtx2,
                            t_names[0],
                            test_stacks[0],
                            STACK_SIZE,
                            1,
                            0,
                            0);
    if (ret != XOS_OK) {
        return ret;
    }

    ret = xos_thread_join(&test_threads[0], &val);

    ret = xos_thread_delete(&test_threads[0]);
    ret = xos_mutex_delete(&mtx2);

    return val;
}


//-----------------------------------------------------------------------------
// Test main function.
//-----------------------------------------------------------------------------
int32_t
mutex_test(void * arg, int32_t unused)
{
    XosTimer timer1;
    int32_t  ret;

    printf("Mutex test starting\n");

    // Run basic checks from thread
    ret = mutex_basic_checks(0);
    if (ret) {
        printf("ERROR: mutex basic checks failed\n");
        return ret;
    }
    printf("### MTX basic test 1 ....................... PASS\n");

    // Run basic checks from timer callback
    if (have_timer) {
        xos_timer_init(&timer1);
        xos_timer_start(&timer1, 1000, 0, mutex_timer_cb, (void *)&ret);

        while (!flag);
        if (ret != XOS_OK) {
            printf("ERROR: mutex basic checks from intr callback failed\n");
            return ret;
        }
    }
    printf("### MTX basic test 2 ....................... PASS\n");

    // Run timeout test if we have a timer
    if (have_timer) {
        ret = mutex_timeout_test();
        if (ret != XOS_OK) {
            printf("ERROR: mutex timeout test failed\n");
            return ret;
        }
    }
    printf("### MTX timeout test ....................... PASS\n");

    // Run thread test with wait mode = priority
    ret = mutex_thread_test(0);
    if (ret != XOS_OK) {
        printf("ERROR: mutex thread test 1 failed\n");
        return ret;
    }
    printf("### MTX thread test 1 ...................... PASS\n");

    // Run thread test with wait mode = fifo
    ret = mutex_thread_test(1);
    if (ret != XOS_OK) {
        printf("ERROR: mutex thread test 2 failed\n");
        return ret;
    }
    printf("### MTX thread test 2 ...................... PASS\n");

#if !USE_MAIN_THREAD
    // This is the controlling thread, exit to terminate
    exit(0);
#endif
    return 0;
}


//-----------------------------------------------------------------------------
// Main.
//-----------------------------------------------------------------------------
int
main()
{
    int32_t ret;

#if defined BOARD
    xos_set_clock_freq(xtbsp_clock_freq_hz());
#else
    xos_set_clock_freq(XOS_CLOCK_FREQ);
#endif

#if USE_MAIN_THREAD
    // This call converts main() into the init thread
    xos_start_main("main", 7, 0);

    // Start timer if available
    if (xos_start_system_timer(-1, TICK_CYCLES) != XOS_OK) {
        have_timer = 0;
    }

    // Run test
    ret = mutex_test(0, 0);
    return ret;
#else
    // Create the main thread
    ret = xos_thread_create(&mutex_test_tcb,
                            0,
                            mutex_test,
                            0,
                            "mutex_test",
                            mutex_test_stk,
                            STACK_SIZE,
                            7,
                            0,
                            0);

    // Start timer after at least one thread has been created
    if (xos_start_system_timer(-1, TICK_CYCLES) != XOS_OK) {
        have_timer = 0;
    }

    // Start scheduling, does not return
    xos_start(0);
    return -1; // should never get here
#endif
}

