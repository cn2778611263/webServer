#ifndef THREADPOOL_H
#define THREADPOOL_H

#include"web.h"
#include "httprequest.h"
struct task_t
{
    int (*func)(void*);
    void* request;
    struct task_t *next;
};

struct threadpool_t
{
    pthread_mutex_t lock;
    pthread_cond_t cond;
    pthread_t *threads;
    task_t *head;
    int thread_count;
    int queue_size;
};
void* pthreadWork(void* arg);
threadpool_t* threadpool_init(int thread_num);
int thread_pool_add(threadpool_t *pool,int (*func)(void *),void* request);

#endif