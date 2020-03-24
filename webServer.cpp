#include "web.h"
#include "http.h"
#include "threadpool.h"
using namespace std;

int main(int argc,char* argv[])
{
    struct sockaddr_in client;
    socklen_t len = 0;
    int rc; int n;int fd;int infd;
    len = sizeof(client);
    string host;int port=0;int r;
    map<string,string> res = mparsecmdline(argc,argv);

    gethost(res,"h",host,"127.0.0.1");
    getport(res,"p",port,8080);

    //初始化线程池
    threadpool_t *pool = threadpool_init(5);
    if(pool ==NULL)
    {
        cout<<"pool create fail"<<endl;
        return -1;
    }
    //创建epoll
    int epfd = epoll_create(5);
    struct epoll_event event;
    struct epoll_event events[10];

    //创建监听端口 并设置为非阻塞
    int listenfd = open_listenfd(host,port);
    rc = make_socket_non_blocking(listenfd);
    if(rc<0)
    {
        cout<<"make_socket_non_blocking error";
        return -1;
    }
    http_request* request = new http_request();
    http_request_init(request,listenfd,epfd);

    //加入到epoll中
    event.data.ptr = (void*)request;
    event.events = EPOLLIN | EPOLLET;
    rc = epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &event);
    if(rc != 0)
    {
        perror("epoll");
        return -1;
    }

    while(1)
    {
        //监听事件
        n = epoll_wait(epfd,events,10,-1);

        for(int i=0;i<n;i++)
        {
            http_request* r = (http_request*)events[i].data.ptr;
            fd = r->fd;
            if(listenfd == fd)
            {
                infd = accept(listenfd,(sockaddr*)&client,&len);
                if (infd < 0) 
                {
                    //socket为非阻塞状态并且当前没有连接存在就会返回EAGAIN或EWOULDBLOCK
                    if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) 
                    {
                         continue;
                    } 
                    else 
                    {
                        perror("accept");
                        return -1;
                    }
                }
                rc = make_socket_non_blocking(infd);
                if(rc<0)
                {
                    cout<<"make_socket_non_blocking error";
                    return -1;
                }
                http_request* temp = new http_request();
                http_request_init(temp,infd,epfd);
                event.data.ptr = (void*)temp;
                event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                rc = epoll_ctl(epfd, EPOLL_CTL_ADD, infd, &event);
                if(rc != 0)
                {
                    perror("epoll");
                    return -1;
                }
            }
            else
            {
                if ((events[i].events & EPOLLERR) ||
                    (events[i].events & EPOLLHUP) ||
                    (!(events[i].events & EPOLLIN))) 
                {
                    cout<< "epoll: fd:"<<infd<<" error,it will close"<<endl; 
                    delete events[i].data.ptr;
                    close(fd);
                    continue;
                }
                else
                {
                    rc  = thread_pool_add(pool,doit,events[i].data.ptr);
                    if(rc < 0)
                    {
                        cout<<" thread_pool_add fail"<<endl;
                    }
                }
                
            }
            
        }
    }
}
