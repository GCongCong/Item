# include<stdio.h>
# include<stdlib.h>
# include<unistd.h>
# include<string.h>
# include<assert.h>
# include<sys/socket.h>
# include<sys/types.h>
# include<arpa/inet.h>
# include<netinet/in.h>
# include<sys/stat.h>
# include<fcntl.h>

char* file[]={"main.c","main.cpp"};
struct Head
{
    int language;
    int file_size;
};

//1.和服务器建立连接
int StartLink()
{
    int sockfd=socket(PF_INET,SOCK_STREAM,0);
    if(sockfd==-1)
    {
        return -1;
    }
    struct sockaddr_in ser;
    memset(&ser,0,sizeof(ser));
    ser.sin_family=AF_INET;
    ser.sin_port=htons(6000);
    ser.sin_addr.s_addr=inet_addr("127.0.0.1");

    int res=connect(sockfd,(struct sockaddr*)&ser,sizeof(ser));

    if(res==-1)
    {
        return -1;
    }
    return sockfd;
}
//2.打印提示信息，选择语言
int ChoiceLanguage()
{
    printf("*******************\n");
    printf("****1 C语言****\n");
    printf("****2 C++  ****\n");
    printf("*******************\n");
    printf("Please input Language Number: ");
    int language=0;
    scanf("%d",&language);
    return language;
}
//3.进程替换，打开系统vim，用户输入代码
void WriteCoding(int flag,int language)
{
    //flag标志用户选择创建新文件还是打开上一次的文件继续编辑。
    //创建新文件需要先删除原有的文件，再vim打开新的，打开上一次的直接vim打开即可
    if(flag==2)
    {
        unlink(file[language-1]);//删除文件，文件在数组的的位置和语言顺序对应
    }
    pid_t pid=fork();
    assert(pid!=-1);
    if(pid==0)
    {
        execl("/usr/bin/vim","/usr/bin/vim",file[language-1],(char*)0);
        printf("execl error\n");
        exit(0);
    }
    else
    {
        wait(NULL);
    }
}//4.将信息发送给服务器
int SendData(int sockfd,int language)
{
    //1.先发送协议内容：语言类型+文件大小
    struct stat st;
    stat(file[language-1],&st);

    if(st.st_size==0)//用户打开文件没有写入数据，就不用发送了
    {
       int  empty=1;
       return empty;
    }

    struct Head head;
    head.language=language;
    head.file_size=st.st_size;
    send(sockfd,&head,sizeof(head),0);

    int fd=open(file[language-1],O_RDONLY);
    while(1)
    {
        char buff[128]={0};
        int n=read(fd,buff,127);
        if(n<=0)
        {
            break;
        }
        send(sockfd,buff,n,0);
    }
    close(fd);
}
//5.收到服务器的信息
void RecvData(int sockfd)
{
    int size;
    recv(sockfd,&size,4,0);//收到协议头
    int num=0;
    printf("\n");
    printf("--------------代码运行结果为：------------\n");
    printf("\n");

    while(1)
    {
        int x=size-num>127?127:size-num;
        char buff[128]={0};
        int n=recv(sockfd,buff,x,0);
        if(n<=0)
        {
            close(sockfd);
            exit(0);
        }
        printf("%s",buff);
        num+=n;
        if(num>=size)
        {
            break;
        }
    }
    printf("\n");
    printf("------------------------------------------\n");
    printf("\n");
}
//6.打印提示信息
int PrintTag()
{
    printf("**********************\n");
    printf("****1 修改代码****\n");
    printf("****2 下一个  ****\n");
    printf("****3 退出    ****\n");
    printf("**********************\n");
    printf("Please input number:");
    int flag=0;
    scanf("%d",&flag);

    return flag;
}
int main()
{
    //1.和服务器建立链接
    int sockfd=StartLink();
    assert(sockfd!=-1);
    //2.打印提示信息，让用户选择语言
    int language=ChoiceLanguage();
    int flag=2;//标志用户下次是打开上一份代码还是编写下一个，开始就是编写下一个，所以为2
    while(1)
    {
        //3.用户输入代码，进程替换，使用系统的vim
        WriteCoding(flag,language);
        int  empty=0;//标志用户是否输入代码
        //4.将选择的语言和代码发送给服务器
        empty=SendData(sockfd,language);
        //5.获取服务器反馈的结果
        if(!empty)//表示文件不为空
        {
            RecvData(sockfd);
        }
        //6.给用户提示，进行下一次的操作
        flag=PrintTag();
        if(flag==3)
        {
            break;
        }
    }
    close(sockfd);
}
