#ifndef THREADING_H

#include "platform.h"

#define THREAD_RUN_INMMEDIATE 0

struct thread_work_queue;

#define THREAD_QUEUE_CALLBACK(name) void name(thread_work_queue * Queue, void * Data)
typedef THREAD_QUEUE_CALLBACK(thread_queue_callback);

struct thread_work_queue_task
{
    thread_queue_callback * Task;
    void * Data;
};

struct thread_work_queue
{
    volatile uint32 TasksPending;
    volatile uint32 TasksCompleted;

    volatile uint32 NextWrite;
    volatile uint32 NextRead;

    thread_work_queue_task Tasks[256];

    void * Semaphore;
};


API void
QueueCompleteTasks(thread_work_queue * Queue);

API void
CreateThreadQueue(thread_work_queue * Queue, uint32 CountThreads);

API void
AddWorkToQueue(thread_work_queue * Queue, thread_queue_callback * Callback,void * Data);

#define THREADING_H
#endif
