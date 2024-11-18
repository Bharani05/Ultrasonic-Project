
// xos_semaphore.c - XOS Semaphore API interface and data structures.

// Copyright (c) 2003-2015 Cadence Design Systems, Inc.
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


#define _XOS_INCLUDE_INTERNAL_
#include "xos.h"


#if XOS_SEM_DEBUG
#define XOS_SEM_SIG     0x73656d61      // Signature of valid object
#define XOS_SEM_SIG_D   0x616d6573      // signature of deleted object
#endif


//-----------------------------------------------------------------------------
//  Initialize a semaphore object before first use.
//-----------------------------------------------------------------------------
int32_t
xos_sem_create(XosSem * sem, uint32_t flags, uint32_t initial_count)
{
    if (sem != XOS_NULL) {
        sem->count = initial_count;
        sem->flags = flags;
        sem->waitq.head = 0;
        sem->waitq.tail = &(sem->waitq.head);

#if XOS_SEM_DEBUG
        sem->sig = XOS_SEM_SIG;
#endif
        return XOS_OK;
    }

    return XOS_ERR_INVALID_PARAMETER;
}


//-----------------------------------------------------------------------------
//  Destroy a semaphore object. Release any waiting threads.
//  In the worst case, this can leave interrupts disabled for quite a while,
//  but since this is expected to be a very infrequent operation, it should
//  be OK. Especially since in normal operation we shouldn't have any waiting
//  threads at delete time.
//-----------------------------------------------------------------------------
int32_t
xos_sem_delete(XosSem * sem)
{
    bool     flag = false;
    uint32_t ps;

    if (sem == XOS_NULL) {
        return XOS_ERR_INVALID_PARAMETER;
    }

#if XOS_SEM_DEBUG
    XOS_ASSERT(sem->sig == XOS_SEM_SIG);
#endif

    // Make sure no one else can change semaphore state (get/put etc.)
    ps = xos_critical_enter();

    if (sem->waitq.head != XOS_NULL) {
        // Release waiting threads
        XosThread * waiter;

        // Don't want unexpected context switching to happen here
        xos_preemption_disable();
        flag = true;

        while ((waiter = xos_q_pop(&sem->waitq)) != 0) {
            xos_thread_wake(waiter, xos_blkon_sem, XOS_ERR_SEM_DELETE);
        }
    }

    // Must do this with the object protected against updates
    sem->count = 0;
    sem->flags = 0;
    sem->waitq.head = 0;
    sem->waitq.tail = 0;
#if XOS_SEM_DEBUG
    sem->sig = XOS_SEM_SIG_D;
#endif

    xos_critical_exit(ps);
    if (flag) {
        // May cause immediate context switch
        xos_preemption_enable();
    }

    return XOS_OK;
}


//-----------------------------------------------------------------------------
//  Decrement the semaphore, block until possible.
//-----------------------------------------------------------------------------
int32_t
xos_sem_get(XosSem * sem)
{
    int32_t  ret = XOS_OK;
    uint32_t ps;

    // Can't call this from interrupt context.
    if (INTERRUPT_CONTEXT) {
        xos_fatal_error(XOS_ERR_INTERRUPT_CONTEXT, 0);
    }

    if (sem == XOS_NULL) {
        return XOS_ERR_INVALID_PARAMETER;
    }
    
#if XOS_SEM_DEBUG
    XOS_ASSERT(sem->sig == XOS_SEM_SIG);
#endif

    ps = xos_critical_enter();
    
    while (ret == XOS_OK) {
        if (sem->count > 0) {
            sem->count--;
            break;
        }

        // Put us on the wait queue and block. Use fifo or priority
        // order as specified.
        ret = xos_block(xos_blkon_sem,
                        &sem->waitq,
                        0,
                        sem->flags & XOS_SEM_WAIT_FIFO ? 0 : 1);
    }

    xos_critical_exit(ps);
    return ret;
}


//-----------------------------------------------------------------------------
//  Timer callback to handle wait timeouts.
//-----------------------------------------------------------------------------
#if XOS_OPT_WAIT_TIMEOUT
static void
sem_wakeup_cb(void * arg)
{
    XosThread * thread = (XosThread *) arg;

    XOS_ASSERT(thread);

    // If the thread is already unblocked or blocked on something else
    // that will be handled inside xos_thread_wake().
    xos_thread_wake(thread, xos_blkon_sem, XOS_ERR_TIMEOUT);
}
#endif


//-----------------------------------------------------------------------------
//  Decrement the semaphore, block until possible or until timeout occurs.
//-----------------------------------------------------------------------------
int32_t
xos_sem_get_timeout(XosSem * sem, uint64_t to_cycles)
{
    int32_t  ret = XOS_OK;
    uint32_t ps;

#if XOS_OPT_WAIT_TIMEOUT
    XosTimer timer;
#endif

    // Can't call this from interrupt context.
    if (INTERRUPT_CONTEXT) {
        xos_fatal_error(XOS_ERR_INTERRUPT_CONTEXT, 0);
    }

    if (sem == XOS_NULL) {
        return XOS_ERR_INVALID_PARAMETER;
    }

#if XOS_SEM_DEBUG
    XOS_ASSERT(sem->sig == XOS_SEM_SIG);
#endif

    ps = xos_critical_enter();

    while (ret == XOS_OK) {
        if (sem->count > 0) {
            sem->count--;
            break;
        }

#if XOS_OPT_WAIT_TIMEOUT
        // If specified and enabled, set up a timeout for the wait
        if (to_cycles > 0) {
            xos_timer_init(&timer);
            ret = xos_timer_start(&timer,
                                  to_cycles,
                                  XOS_TIMER_DELTA | XOS_TIMER_FROM_NOW,
                                  &sem_wakeup_cb,
                                  xos_curr_threadptr);
        }
#endif

        // Put us on the wait queue and block. Use fifo or priority
        // order as specified.
        ret = xos_block(xos_blkon_sem,
                                 &sem->waitq,
                                 0,
                                 sem->flags & XOS_SEM_WAIT_FIFO ? 0 : 1);
    }

#if XOS_OPT_WAIT_TIMEOUT
    if (to_cycles > 0) {
        xos_timer_stop(&timer);
    }
#endif

    xos_critical_exit(ps);
    return ret;
}


//-----------------------------------------------------------------------------
//  Increment the semaphore count.
//  This will also wake up one waiting thread, if there is one. Note that the
//  woken thread may not actually complete its wait, because by the time it
//  gets to run, it is possible that another thread has decremented the count
//  already. In that case the woken thread will go back to being blocked.
//-----------------------------------------------------------------------------
int32_t
xos_sem_put(XosSem * sem)
{
    int32_t     ret = XOS_OK;
    uint32_t    ps;
    XosThread * waiter;

    if (sem == XOS_NULL) {
        return XOS_ERR_INVALID_PARAMETER;
    }

#if XOS_SEM_DEBUG
    XOS_ASSERT(sem->sig == XOS_SEM_SIG);
    XOS_ASSERT(sem->count < UINT32_MAX);
#endif

    ps = xos_critical_enter();

    sem->count++;
    waiter = xos_q_pop(&sem->waitq);
    if (waiter != XOS_NULL)
        xos_thread_wake(waiter, xos_blkon_sem, 0);

    xos_critical_exit(ps);
    return ret;
}


//-----------------------------------------------------------------------------
//  Try to decrement the semaphore, but do not block if failed.
//-----------------------------------------------------------------------------
int32_t
xos_sem_tryget(XosSem * sem)
{
    int32_t  ret = XOS_OK;
    uint32_t ps;

    if (sem == XOS_NULL) {
        return XOS_ERR_INVALID_PARAMETER;
    }

#if XOS_SEM_DEBUG
    XOS_ASSERT(sem->sig == XOS_SEM_SIG);
#endif

    ps = xos_critical_enter();

    if (sem->count > 0) {
        sem->count--;
    }
    else {
        // Can't take, return
        ret = XOS_ERR_SEM_BUSY;
    }

    xos_critical_exit(ps);
    return ret;
}

