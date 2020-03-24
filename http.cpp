#include"http.h"

// struct http_request
// {
//     int fd;
//     int epfd;
//     int status;//状态机中的状态设置
//     char buf[4096];
//     char* method;
//     char* uri;
//     char* version;
//     int index;//当前读取到的位置
//     int pos;//开始位置
//     bool keep_alive;
// };


int doit(void* request)
{
    http_request * req = (http_request*)request;
    int len = 0;
    int remain = 0;int rc = 0;
    while(1)
    {
        remain = MAXBUF - req->end;
        len = read(req->fd,req->buf+req->end,remain);
        if(len == 0)
        {
            cout<< "remote close ,the fd "<<req->fd<<"will close"<<endl;
            close(req->fd);
            delete req;
            return -1;
        }
        else if(len == -1)
        {
            if(errno != EAGAIN)
            {
                cout<< "not error EAGAIN, fd will close"<<endl;
                close(req->fd);
                delete req;
                return -1;
            }
            break;
        }

        req->end += len;
        if(req->status == HEAD)
        {
            if((rc = http_prase_request_line(req)) != 0)
            {
                continue;
            }
        }

        if(req->status == BODY)
        {
            if((rc = http_prase_request_body(req)) != 0)
            {
                continue;
            }
        }   
        //string temp = req->method;
        //cout<<temp<<".."<<endl;
        string temp = req->method;
        if(temp != "GET")
        {
            clienterror(req->fd,req->method,(char*)"501",(char*)"Not implemented",(char*)"Web does not implement this method");
            return 0;
        }

        struct stat stat_buf;
        string filename,cgiargs;

        int is_static = prase_uri(req->uri,filename,cgiargs);

        if(stat(filename.c_str(),&stat_buf) < 0)
        {
            clienterror(req->fd,(char*)filename.c_str(),(char*)"404",(char*)"Not found",(char*)"Tiny counldn t find this file");
            return 0;
        }

        if(is_static)
        {
            if(!S_ISREG(stat_buf.st_mode) || !(S_IRUSR & stat_buf.st_mode))
            {
                clienterror(req->fd,(char*)filename.c_str(),(char*)"403", (char*)"Forbidden",(char*)"Tiny couldn't read the file");
                return 0;
            }

            server_static(req->fd,filename,stat_buf.st_size);
        }
        else
        {
            if(!S_ISREG(stat_buf.st_mode) || !(S_IXUSR & stat_buf.st_mode))
            {
                clienterror(req->fd,(char*)filename.c_str(),(char*)"403", (char*)"Forbidden",(char*)"Tiny couldn't read the file");
                return 0;           
            }

            server_dynamic(req->fd,filename,cgiargs);
        }       
    }
    return 0;
}



void get_filetype(string filename,string& filetype)
{
    if(filename.find(".html"))
    {
        filetype = "text/html";
    }
    else if(filename.find(".gif"))
    {
        filetype = "image/gif";
    }
    else if(filename.find(".jpeg"))
    {
        filetype = "image/jpeg";
    }
    else
    {
        filetype = "text/plain";
    }
    return;
}



void server_static(int fd,string filename,int filesize)
{
    char buf[MAXSIZE];
    string filetype;
    int filefd;

    get_filetype(filename,filetype);

    sprintf(buf,"HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype.c_str());
    write(fd,buf,strlen(buf));

    bzero(buf,sizeof(buf));

    filefd = open(filename.c_str(),O_RDONLY,0);
    while(read(filefd,buf,MAXSIZE))
    {
        write(fd,buf,strlen(buf));
        bzero(buf,sizeof(buf));
    }
    return;
}




void server_dynamic(int fd,string filename,string cgiargs)
{
    char buf[MAXSIZE];
    char* argv[] = {NULL};

    sprintf(buf,"HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    write(fd,buf,strlen(buf));

    int pid = fork();

    if(pid == 0)
    {
        setenv("QUERY_STRING",cgiargs.c_str(),1);
        dup2(fd,STDOUT_FILENO);
        execve(filename.c_str(),argv,environ);
    }
    wait(NULL);
    return;
}



int prase_uri(string uri,string& filename,string& cgiargs)
{
    if(uri.find("cgi-bin")==string::npos)
    {
        cgiargs = "";
        if(uri=="/")
        {
            filename = "./index.html";
        }
        else
        {
            filename = "."+uri;
        }
        return 1;
    }
    else
    {
        int index = uri.find_first_of('?');
        if(index != string::npos)
        {
            cgiargs = uri.substr(index+1);
            filename =  "." + uri.substr(0,index);
        }
        else
        {
            cgiargs = "";
            filename = "." + uri;
        }
        
        return 0;
    }
    
}



void clienterror(int fd,char* cause,char* errnum,char* shortmsg,char* longmsg)
{
    char buf[MAXSIZE],body[MAXSIZE];

    sprintf(body, "<html><title>WebServer Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Web server</em>\r\n", body);

    sprintf(buf,"HTTP/1.0 %s %s\r\n",errnum,shortmsg);
    write(fd,buf,strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    write(fd,buf,strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    write(fd,buf,strlen(buf));
    write(fd,body,strlen(body));
}



int http_prase_request_line(http_request* req)
{
    string s = req->buf;
    string head;
    int flag = 0;
    cout<<s<<"..."<<endl;
    if((flag = s.find("\r\n"))!= string::npos)
    {
        head = s.substr(0,flag+7);
        char temp[MAXSIZE];int i =0;
        for(i=0;i<head.size();i++)
        {
            temp[i] = head[i];
        }
        temp[i] = '\0';
        sscanf(temp,"%s %s %s",req->method,req->uri,req->version);
        req->status = BODY;
        req->pos = flag + 2;
        return 0;
    }
    else
    {
        return 1;
    }
    
}



int http_prase_request_body(http_request* req)
{
    char temp_buf[MAXBUF];
    memset(temp_buf, '\0', sizeof(temp_buf));
    strncpy(temp_buf,req->buf+req->pos,req->end-req->pos+1);
    string s = temp_buf;
    int flag = 0;
    if((flag = s.find("\r\n\r\n"))!=string::npos)
    {
        if((flag = s.find("Keep-Alive"))!=string::npos)
        {
            req->keep_alive = true;
        }
        else
        {
            req->keep_alive = false;
        }
        req->status = HEAD_BODY_OK;
        return 0;
    }
    return -1;
}

