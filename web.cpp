#include "web.h"
//存放webServer使用的函数
map<string,string> mparsecmdline(int argc,char* argv[])
{
    map<string,string> rec;
    int i=1; char* cur;
    while(i<argc)
    {
        cur = argv[i];
        if(strlen(cur) < 2 || cur[0]!= '-')
        {
            cout<<"未识别参数"<<endl;
            exit(1);
        }
        ++cur;
        if(i<argc-1 && argv[i+1][0] != '-')
        {
            rec[string(cur)] = string(argv[i+1]);
            i += 2;
        }
        else
        {
            rec[string(cur)] = "true";
            i++;
        }
    }
    return rec;
}

void gethost(map<string,string> res,string flag,string &rec,string str)
{
    if(res.find(flag)!=res.end())
    {
        rec = res[flag];
    }
    else
    {
        rec = str;
    }
    return;  
}

void getport(map<string,string> res,string flag,int &rec,int def)
{
    if(res.find(flag)!=res.end())
    {
        rec = stoi(res[flag]);
    }
    else
    {
        rec = def;
    }
    return;
}
int open_listenfd(string shost,int port)
{
    int r;
    char* host = (char*)shost.c_str();
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(short(port));
    serveraddr.sin_addr.s_addr = inet_addr(host);
    //inet_aton(host,&serveraddr.sin_addr);

    int listenfd = socket(AF_INET,SOCK_STREAM,0);
    if(listenfd < 0)  ERR_RETUNRE("listenfd")
    int onoff = 1;
    int ret = setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&onoff,sizeof(onoff));
    if(ret<0) ERR_RETUNRE("ret")
    r = bind(listenfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr));
    if(r < 0)  ERR_RETUNRE("bind")

    r = listen(listenfd,5);
    if(r < 0) ERR_RETUNRE("listen")
    cout<<"listening"<<endl;
    return listenfd;
}

int make_socket_non_blocking(int fd) {
    int flags, s;
    flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        return -1;
    }

    flags |= O_NONBLOCK;
    s = fcntl(fd, F_SETFL, flags);
    if (s == -1) {
        return -1;
    }

    return 0;
}