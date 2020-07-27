#define _GNU_SOURCE
# include<stdio.h>
# include<stdlib.h>
# include<string.h>
# include<unistd.h>
# include<assert.h>
# include<sys/types.h>
# include<sys/socket.h>
# include<arpa/inet.h>
# include<netinet/in.h>
# include<sys/epoll.h>
# include<fcntl.h>
# include<errno.h>
# include<sys/stat.h>

# define MAXEVENTS 100
char* file[]={"main.c","main.cpp"};//创建的文件种类，和语言类型序号对应
char* build[]={"/usr/bin/gcc","/usr/bin/g++"};//系统自带的编译器存储路径
char* carry[]={"./a.out","./a.out"};//执行编译后的文件指令
//自定协议，每次服务器，客户端交互时先发送这个,解决粘包问题
struct Head
{
    int language;
    int file_size;
};

//1. 创建监听套接字,记得对每个函数返回值判断socket(),bind(),listen()
int CreateSocket()
{
    int sockfd=socket(PF_INET,SOCK_STREAM,0);//底层协议，服务类型，默认协议标识
    if(sockfd==-1)
    {
        return -1;
    }
    struct sockaddr_in ser_addr;
    memset(&ser_addr,0,sizeof(ser_addr));
    ser_addr.sin_family=AF_INET;//地址族
    ser_addr.sin_port=htons(6000);//主机字节序转网络字节序
    ser_addr.sin_addr.s_addr=inet_addr("127.0.0.1");//点分十进制转网络字节序标准IPV4地址

    int n=bind(sockfd,(struct sockaddr*)&ser_addr,sizeof(ser_addr));//记得将第二个参数强转为sockaddr*类型，绑定
    if(n==-1)
    {
        return -1;
    }
    n=listen(sockfd,5);//第二个参数为内核维护的完成三次握手的连接
    if(n == -1)
    {
        return -1;
    }
    return sockfd;
}
//2-1获取新客户端连接accept
void GetNewClient(int sockfd,int epfd)
{
    struct sockaddr_in cli;//将连接的客户端信息保存在这里
    socklen_t len=sizeof(cli);
    int fd=accept(sockfd,(struct sockaddr*)&cli,&len);

    if(fd<0)
    {
        return;
    }
    printf("客户端%d已连接\n",fd);

    struct epoll_event event;//定义事件类型结构体，将客户端信息保存
    event.data.fd=fd;
    event.events=EPOLLIN | EPOLLRDHUP|EPOLLET;//监听客户端的读事件，异常断开事件,采取ET模式
    //添加到内核事件表中
    epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&event);
    //将监听的文件描述设置为非阻塞，否则会阻塞
    int flag=fcntl(fd,F_GETFL);//获得文件描述符状态
    flag=flag|O_NONBLOCK;//将状态设置为非阻塞
    fcntl(fd,F_SETFL,flag);//将非阻塞状态设置给文件描述符
}
//2-2-1 接受客户端数据,返回用户传递的语言类型
int RecvCoding(int fd)
{
    //1.接收协议头，语言类型+文件大小，根据语言类型创建对应文件
    struct Head head;
    recv(fd,&head,sizeof(head),0);
    int filefd=open(file[head.language-1],O_WRONLY|O_TRUNC|O_CREAT,0664);//根据传来的语言类型打开对应的文件
    //2.接受代码
    int size=0;
    while(1)
    {
        int num=head.file_size-size>127?127:head.file_size-size;//确定收数据的大小，防止最后一次发生粘包问题
        char buff[128]={0};
        int n=recv(fd,buff,num,0);
        if(n==0)
        {
            break;
        }
        if(n==-1)
        {
            if(errno==EAGAIN || errno==EWOULDBLOCK)//表示缓冲区中没有数据可读，唤醒sockfd进行下一次读操作，如果不设置，文件描述符一直等着数据到来
            {
                printf("Ser read over\n");
                break;
            }
        }
        else
      {
        size+=n;
        write(filefd,buff,n);//将收到的数据存储到文件中
        if(size>=head.file_size)
        {
            break;
        }
     }
    }
    close(filefd);
    return head.language;//返回语言类型，根据语言类型进行编译
}
//2-2-2编译代码,替换为系统自带的编译器进行编译
int BuildCoding(int language)
{
    struct stat st;//定义存储文件信息的结构体
    pid_t pid=fork();//创建子进程
    assert(pid!=-1);
    if(pid==0)//子进程替换为自带编译器,将编译结果写入错误文件，所以根据文件大小可以判定是否编译成功，文件大小为0，编译成功，大于0，表示有错误信息写入，编译失败
    {
        int fd=open("./build_error.txt",O_CREAT|O_WRONLY|O_TRUNC,0664);
        close(1);//关闭标准输入
        close(2);//关闭标准错误输入
        dup(fd);//将文件描述符重定位？？为啥从1重定位
        dup(fd);
        execl(build[language-1],build[language-1],file[language-1],(char*)0);//进程替换
        write(fd,"build error",11);//替换失败，将其写入
        exit(0);
    }
    else
    {
        wait(NULL);//阻塞等待子进程结束
        stat("./build_error.txt",&st);//将文件大小放到st结构中
    }
    return st.st_size;//返回错误文件大小，0表示编译成功，>0表示编译失败
}
//2-2-3 执行代码,替换为./a.out程序即可,将执行结果保存在文件中
void Carry(int language)
{
    pid_t pid=fork();
    assert(pid!=-1);
    if(pid==0)
    {
        int fd=open("./result.txt",O_WRONLY|O_TRUNC|O_CREAT,0664);
        close(1);
        close(2);
        dup(fd);
        dup(fd);
        execl(carry[language-1],carry[language-1],(char*)0);
        write(fd,"carry error",11);//替换失败，写入结果
        exit(0);
    }
    else
    {
        wait(NULL);
    }

}
//2-2-4给客户端发送结果，根据标志决定发送错误文件还是执行文件，先发送文件大小
void SendResult(int fd,int flag)
{
    char* file="./result.txt";
    if(flag)
    {
        file="./build_error.txt";
    }
    struct stat st;
    stat(file,&st);

    send(fd,(int*)&st.st_size,4,0);
    int filefd=open(file,O_RDONLY);
    while(1)
    {
        char buff[128]={0};
        int n=read(filefd,buff,127);
        if(n<=0)
        {
            break;
        }
        send(fd,buff,n,0);
    }
    close(filefd);
}
//2-2处理客户端描述符就绪
void DealClientData(int fd)
{
    //2-2-1接受客户端的数据，将代码存储到本地文件
    int language=RecvCoding(fd);
    //2-2-2编译代码，将编译结果存储到编译错误文件
    int flag=BuildCoding(language);//flag标志编译是否出现错误
    //2-2-3执行代码，编译成功才可以运行
    if(flag==0)
    {
        Carry(language);//执行代码，结果存储到文件中
        //2-2-4 给客户端发送信息
        SendResult(fd,flag);//将执行结果文件发送
    }
    else
    {
        SendResult(fd,flag);//发送编译错误文件
    }
}

//2.处理数据
void DealFinishEvents(int sockfd,int epfd,struct epoll_event* events,int num)
{
    int i=0;
    for(i;i<num;++i)//循环处理就绪事件
    {
        int fd=events[i].data.fd;//获取就绪事件描述符
        //2-1.新的客户端连接，监听套接字socket就绪
        if(fd==sockfd)
        {
            //获取新客户端连接
            GetNewClient(sockfd,epfd);
        }
        else //2-2.客户端连接文件描述符上有事件就绪
        {
            // 客户端断开连接事件
            if(events[i].events & EPOLLRDHUP)
            {
                //关闭文件描述符
                //从epoll的内核事件表中删除该文件描述符
                printf("客户端%d断开连接\n",fd);
                close(fd);
                epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL);
            }
            else//客户端发送数据，处理数据
            {
                DealClientData(fd);
            }

        }

    }
}
int main()
{
    int sockfd=CreateSocket();
    assert(sockfd!=-1);
    //创建内核事件表，将监听套接字添加
    int epfd=epoll_create(5);
    struct epoll_event event;
    event.data.fd=sockfd;
    event.events=EPOLLIN;//监听读事件
    epoll_ctl(epfd,EPOLL_CTL_ADD,sockfd,&event);//添加到内核事件表中

    while(1)
    {
        struct epoll_event events[MAXEVENTS];//存储内核返回的就绪事件
        int n=epoll_wait(epfd,events,MAXEVENTS,-1);//监听内核事件表
        if(n<=0)
        {
            printf("epoll_wait error\n");
            continue;
        }
        DealFinishEvents(sockfd,epfd,events,n);//处理就绪事件函数
    }

}
