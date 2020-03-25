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
    //用于解析提供的参数 ./webServer -h 127.0.0.1 -p 8080
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

    //创建监听端口 并设置为非阻塞 如果设置失败则返回错误
    int listenfd = open_listenfd(host,port);
    rc = make_socket_non_blocking(listenfd);
    if(rc<0)
    {
        cout<<"make_socket_non_blocking error";
        return -1;
    }
    //所有的套接字都封装为request类型，并初始化
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
        //有事件返回 遍历所有返回的事件
        for(int i=0;i<n;i++)
        {
            http_request* r = (http_request*)events[i].data.ptr;
            fd = r->fd;
            //如果返回的事件是listenfd就建立连接
            if(listenfd == fd)
            {
                //listenfd是非阻塞的，为了处理客户端异常关闭情况
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
                //之所以设置EPOLLONESHOT是为了防止多个线程同时操作一个fd
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
                //如果监听到了对端关闭或错误 就关闭fd
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
                    //当有客户端向服务器发送了HTML请求报文
                    //将其交给线程
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
