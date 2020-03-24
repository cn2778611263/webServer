#include "threadpool.h"

threadpool_t* threadpool_init(int thread_num)
{
    if(thread_num <= 0)
    {
        return nullptr;
    }
    //threadpool_t * pool = (threadpool_t*)malloc(sizeof(threadpool_t));
    threadpool_t *pool = new threadpool_t();

    pool->thread_count = 0;
    pool->queue_size = 0;

    //pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_num);
    pool->threads = new pthread_t[thread_num];
    
    pool->head = new task_t();//dummyHead
    pool->head->func = NULL;
    pool->head->next = NULL;
    pool->head->request = NULL;
    
    if(pthread_mutex_init(&pool->lock,NULL) != 0)
    {
        return nullptr;
    }

    if(pthread_cond_init(&pool->cond,NULL) != 0)
    {
        return nullptr;
    }

    for(int i=0;i<thread_num;i++)
    {
        if(pthread_create(&pool->threads[i],NULL,pthreadWork,(void *)pool) != 0)
        {
            delete pool;
            return nullptr;
        }
        pool->thread_count++;
    }
    return pool;
}
//此函数功能是将任务加入到工作队列，并唤醒阻塞线程
int thread_pool_add(threadpool_t *pool,int (*func)(void *),void* request)
{
    int rc = 0;
    if(pool == NULL || request == NULL)
    {
        cout<<"zheli?"<<endl;
        return -1;
    }
    if(pthread_mutex_lock(&pool->lock)!=0)
    {
        cout<<"lock fail"<<endl;
        return -1;
    }

    task_t * task = new task_t();
    task->func = func;
    task->request = request;
    task->next = pool->head->next;
    pool->head->next = task;

    pool->queue_size++;

    pthread_cond_signal(&pool->cond);
    if(pthread_mutex_unlock(&pool->lock)!=0)
    {
        cout<<"unlock fail"<<endl;
        return -1;
    }
    return 0;
}

void* pthreadWork(void* arg)
{
    threadpool_t *pool = (threadpool_t *)arg;

    pthread_mutex_lock(&pool->lock);
    while(1)
    {
        while(pool->queue_size == 0)
        {
            pthread_cond_wait(&pool->cond,&pool->lock);
        }
        if(pool->head->next==nullptr)
        {
            pthread_mutex_unlock(&pool->lock);
            continue;
        }
        pool->queue_size--;
        task_t * worker = pool->head->next;
        pool->head->next = pool->head->next->next;

        pthread_mutex_unlock(&pool->lock);

        int rc = (*(worker->func))(worker->request);
        if(rc != -1)
        {
            http_request *temp = (http_request*)worker->request;
            if(temp->status)
            {
                struct epoll_event event;
                event.data.ptr = worker->request;
                event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                epoll_ctl(temp->epfd, EPOLL_CTL_MOD,temp->fd, &event);
            }
            else
            {
                close(temp->fd);
                delete temp;
            }
        }
    }
}
