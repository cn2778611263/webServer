# webServer
应用epoll+非阻塞IO+线程池模型实现的简易服务器，可以实现动态服务和静态服务。


webServer.cpp：整个工程的入口，负责初始化线程池，epoll以及对socket描述符的监听

web.cpp：封装一些webServer所需函数

threadpool.cpp:包括初始化线程池函数，向线程池添加任务函数以及线程的工作函数

http.cpp:在收到HTTP请求后具体如何解析，如何发送HTTP应答

httprequest.cpp/.h:所有socke描述符都封装为request类型，并初始化

采用线程池的好处：
  多进程并发服务器并发度不高，面临极其耗费资源的进程销毁、创建和切换。多线程并发是一种选择，但是多线程同样面临线程频繁的创建和销毁，
  因此采用线程池的方法可以避免这一点。
  
  线程池的工作方式：首先创建若干线程，使其进入线程工作函数，在消费者生产者队列没用任务的情况下进入阻塞等待状态，一旦有任务发生便会被
  唤醒执行任务，任务执行完后再次进入阻塞状态等待其他任务。
  
采用非阻塞IO的原因：
  HTTP/1.1中客户端和服务器默认是长连接的，因此如果有n个线程，来n+1个长连接还是无法处理，采用非阻塞IO的方式，即使是长连接，只有有数据的时候线
  程才去会处理，没有数据直接返回。
  
学到的东西：

      1.Web服务器工作原理 
   
      2.线程池的创建和使用
   
      3.状态机
   
      4.非阻塞IO处理长连接
   
      5.gdb进行调试
   
      6.tcpdump抓包调试（简直太有用了）
   
   
不足：

      1.未实现应用层的定时器来处理长期无请求的情况
   
      2.只支持GET方法
   
   
   WebServer.......（未完待续）
