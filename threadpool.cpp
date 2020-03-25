#include "threadpool.h"

threadpool_t* threadpool_init(int thread_num)
{
    if(thread_num <= 0)
    {
        return nullptr;
    }
    //threadpool_t * pool = (threadpool_t*)malloc(sizeof(threadpool_t));
    //分配一个线程池对象并对其进行初始化
    threadpool_t *pool = new threadpool_t();
    
    pool->thread_count = 0;
    pool->queue_size = 0;

    //pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_num);

    //分配thread_num个空间存储线程tid
    pool->threads = new pthread_t[thread_num];
    
    pool->head = new task_t();//dummyHead
    pool->head->func = NULL;
    pool->head->next = NULL;
    pool->head->request = NULL;
    //对锁进行初始化
    if(pthread_mutex_init(&pool->lock,NULL) != 0)
    {
        return nullptr;
    }
    //对条件变量进行初始化
    if(pthread_cond_init(&pool->cond,NULL) != 0)
    {
        return nullptr;
    }

    for(int i=0;i<thread_num;i++)
    {
        //创建4个线程 它们的工作函数是pthreadWork
        if(pthread_create(&pool->threads[i],NULL,pthreadWork,(void *)pool) != 0)
        {
            //如果创建失败，销毁线程池
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
        return -1;
    }
    if(pthread_mutex_lock(&pool->lock)!=0)
    {
        cout<<"lock fail"<<endl;
        return -1;
    }
    //初始化一个任务，将其加入到任务队列里面
    task_t * task = new task_t();
    task->func = func;
    task->request = request;
    task->next = pool->head->next;
    pool->head->next = task;

    pool->queue_size++;
    //唤醒一个阻塞的线程
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
    while(1)
    {
        //操作pool->queue_size之前要获得锁
        pthread_mutex_lock(&pool->lock);
        while(pool->queue_size == 0)
        {
            //如果没有要处理的任务就阻塞
            pthread_cond_wait(&pool->cond,&pool->lock);//释放锁
        }
        if(pool->head->next==nullptr)
        {
            pthread_mutex_unlock(&pool->lock);
            continue;
        }
        //取出一个任务
        pool->queue_size--;
        task_t * worker = pool->head->next;
        pool->head->next = pool->head->next->next;

        pthread_mutex_unlock(&pool->lock);
        //执行对应的任务函数
        int rc = (*(worker->func))(worker->request);
        if(rc != -1)
        {
            http_request *temp = (http_request*)worker->request;
            if(temp->status==0 || temp->status==1)
            {
                struct epoll_event event;
                event.data.ptr = worker->request;
                event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                epoll_ctl(temp->epfd, EPOLL_CTL_MOD,temp->fd, &event);                
            }
            if(temp->status == 3)
            {
                if(temp->keep_alive)
                {
                    memset(temp->buf,'\0',sizeof(temp->buf));
                    memset(temp->method,'\0',sizeof(temp->method));
                    memset(temp->uri,'\0',sizeof(temp->uri));
                    memset(temp->version,'\0',sizeof(temp->version));
                    temp->end = 0;
                    temp->pos=0;
                    temp->status = 0;
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
}
