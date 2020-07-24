# include<stdio.h>
# include<unistd.h>
# include<string.h>
# include<stdlib.h>
# include<assert.h>
# include<sys/types.h>//getpwuid
# include<pwd.h>//getpwuid
# include<sys/utsname.h>//uname

# define SIZE 128
# define NUM 20
//1.打印终端信息
void PrintfMessage()
{
    struct passwd* pw=getpwuid(getuid());//获取指向用户信息结构体的指针
    assert(pw!=NULL);
    struct utsname uts;//获取主机信息结构体
    uname(&uts);
    char path[SIZE]={0};//保存当前绝对目录
    getcwd(path,127);//获取
    char* dir=NULL;//保存最后目录
    //家目录
    if(strcmp(path,pw->pw_dir)==0)
    {
        dir="~";
    }
    //普通目录和/目录
    else
    {
        char* p=path+strlen(path);//指向文件末尾
        while(*p!='/')
        {
            p--;
        }
        if(strlen(path)!=1)//等于1表示为/，不用处理，直接输出
        {
            p++;
        }
        dir=p;
    }
    char symbol='$';
    if(getuid()==0)
    {
        symbol='#';
    }

    printf("[%s@%s %s]%c ",pw->pw_name,uts.nodename,dir,symbol);

}
//2.分割字符串
void CutCommand(char* com,char* comarr[])
{
    int index=0;
    char* p=strtok(com," ");
    while(p!=NULL && index<NUM)
    {
        comarr[index++]=p;
        p=strtok(NULL," ");
    }
}
//3.实现cd内置命令
void DealCd(char* path)
{
    static char oldpath[SIZE]={0};
     //1.实现cd -到达上一层路径
    if(strncmp(path,"-",1)==0)
    {
        if(strlen(oldpath)==0)
        {
            printf("Cd error,No OLDPATH!\n");
            return;
        }

        path=oldpath;
    }
    //2.实现cd ~到达家目录
    else if(strncmp(path,"~",1)==0||path==NULL)
    {
        struct passwd* pw=getpwuid(getuid());
        assert(pw!=NULL);
        path=pw->pw_dir;
    }
     //3.调用切换目录函数
    char nowpath[SIZE]={0};
    getcwd(nowpath,SIZE-1);
    if(chdir(path)==-1)
    {
        perror("cd");
        return;
    }
    memset(oldpath,0,SIZE);
    strcpy(oldpath,nowpath);
}
//4.实现外置命令
void Dealoutcmd(char* comarr[])
{
        pid_t pid=fork();
        assert(pid!=-1);
        if(pid==0)
        {
            char file[SIZE]={0};
            //如果用户输入的命令中给出了路径
            if(strstr(comarr[0],"/")!=NULL)
            {
                strcpy(file,comarr[0]);
            }
            //用户只输入了命令
            else
            {
                strcpy(file,"/home/Gripure/Desktop/review/mybash/Mybin/");
                strcat(file,comarr[0]);
            }
            execv(file,comarr);
        }
        else
        {
            wait(NULL);
        }
}
int main()
{
    while(1)
    {
        //1.输出终端提示信息
        PrintfMessage();
        //2.获取
        char com[SIZE]={0};
        fgets(com,127,stdin);
        com[strlen(com)-1]=0;
        //3.处理空命令
        if(strlen(com)==0)
        {
            continue;
        }
        //4.将命令和参数分割        
 	    char* comarr[NUM]={0};
        CutCommand(com,comarr);
        //5.处理内置命令
        //5.1 实现Cd命令
        if(strlen(comarr[0])==2 && strncmp(comarr[0],"cd",2)==0)
        {
            DealCd(comarr[1]);
        }
        //5.2 实现exit命令
        else if(strlen(comarr[0])==4 && strncmp(comarr[0],"exit",4)==0)
        {
            exit(0);
        }
        //6.处理外置命令
        else
        {
            Dealoutcmd(comarr);
        }
        
    }
}
