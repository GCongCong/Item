# include<stdio.h>
# include<stdlib.h>
# include<unistd.h>
# include<string.h>
# include<assert.h>
# include<sys/types.h>//opendir,readdir,closedir
# include<dirent.h>
# include<sys/stat.h>

# define OPTION_A 0  //a
# define OPTION_I 1  //i
# define OPTION_L 2  //l

int option=0;//按位保存传入的选项
# define SetOption(option,val) option|=(1<<val)//设置选项
# define GetOption(option,val) option&(1<<val)//获得选项

//1.判断选项，保存到option中
void JugeArg(int argc,char* argv[])
{
    int i=1;
    for(;i<argc;i++)
    {
        if(strncmp(argv[i],"-",1)!=0)//没有选项，直接下一次判断
        {
            continue;
        }
        if(strstr(argv[i],"a")!=NULL)//有a选项
        {
            SetOption(option,OPTION_A);
        }
        else if(strstr(argv[i],"i")!=NULL)//有i选项
        {
            SetOption(option,OPTION_I);
        }
        else if(strstr(argv[i],"l")!=NULL)//有l选项
        {
            SetOption(option,OPTION_L);
        }
    }
}
//2-1 打印带有颜色的文件名 
void PrintName(char* path,char* file)
{
    char filename[128]={0};
    strcpy(filename,path);
    strcat(filename,"/");
    strcat(filename,file);

    struct stat st;
    stat(filename,&st);

    //蓝色目录文件，黑色普通文件，绿色可执行文件
    if(S_ISDIR(st.st_mode))//判断文件类型
    {
        printf("\033[1;34m%s\033[0m  ",file);
    }
    if(S_ISREG(st.st_mode))
    {
        if(st.st_mode & S_IXUSR||
           st.st_mode & S_IXGRP || st.st_mode & S_IXOTH)//判断是否为可执行文件
        {
            printf("\033[1;32m%s\033[0m  ",file);

        }
        else
            printf("%s  ",file);
    }
}
//2-2 实现ls -l
//2-2-1 输出文件类型
void PrintType(struct stat st)
{
    if(S_ISDIR(st.st_mode))
        printf("d");
    else
        printf("-");
}
//2-2-2 输出文件权限
void PrintMode(struct stat st)
{
    if(st.st_mode & S_IRUSR)
        printf("r");
    else
        printf("-");
    if(st.st_mode & S_IWUSR)
        printf("w");
    else
        printf("-");
    if(st.st_mode & S_IXUSR)
        printf("x");
    else
        printf("-");
    printf(" ");
}
void PrintMore(char* path,char* file)
{
    char filename[128]={0};
    strcpy(filename,path);
    strcat(filename,"/");
    strcat(filename,file);
    
    struct stat st;
    stat(filename,&st);

    PrintType(st);
    PrintMode(st);
    printf("%d ",st.st_nlink);
}
//2 根据选项输出文件信息
void PrintFile(char* path)
{
    DIR* dp;
    struct dirent *dt;
    dp=opendir(path);//打开目录流
    int flag=0;
    while((dt=readdir(dp))!=NULL)
    {
        //无a选项，但文件是隐藏文件，直接跳过
        if((!GetOption(option,OPTION_A)) && strncmp(dt->d_name,".",1)==0)
        {
            continue;
        }
        //i选项
        if(GetOption(option,OPTION_I))
        {
            printf("%d ",dt->d_ino);//输出文件节点
        }
        //l选项
        if(GetOption(option,OPTION_L))
        {
            PrintMore(path,dt->d_name);//进行更详细信息的输出
            flag=1;
        
        }
        PrintName(path,dt->d_name);//输出带颜色的文件名
        if(flag)
        {
            printf("\n");
        }
    }
    closedir(dp);
    if(!flag)
    {
        printf("\n");
    }
}
int main(int argc,char* argv[])
{
    JugeArg(argc,argv);
    int flag=0;//标志是否传入路径
    int i=1;
    for(;i<argc;i++)
    {
        if(strncmp(argv[i],"-",1)==0)//为选项，跳过
        {
            continue;
        }
        PrintFile(argv[i]);//输出文件名
        flag=1;
    }
    if(!flag)//没有传入路径，输出当前路径下的文件
    {
        char path[128]={0};
        getcwd(path,127);//获取当前目录地址
        PrintFile(path);
    }
}
