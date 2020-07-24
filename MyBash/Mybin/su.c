# include<stdio.h>
# include<stdlib.h>
# include<unistd.h>
# include<string.h>
# include<assert.h>
# include<sys/types.h>//opendir,readdir,closedir
# include<dirent.h>
# include<sys/stat.h>
# include<termios.h>
# include<shadow.h>
# include<pwd.h>

int main(int argc,char* argv[])
{
    char password[128]={0};
    char* user="root";
    if(argv[1]!=NULL)
    {
        user=argv[1];
    }
    printf("Password:");
    //1. 取消回显
    struct termios oldter,newter;
    tcgetattr(0,&oldter);
    newter=oldter;
    newter.c_lflag &= ~ECHO;
    tcsetattr(0,TCSANOW,&newter);
    fgets(password,127,stdin);
    tcsetattr(0,TCSANOW,&oldter);
    password[strlen(password)-1]=0;
    //2.获得用户在系统中的密码，分割得到加密算法和密钥
    struct spwd* sp=getspnam(user);
    assert(sp!=NULL);
    char* p=sp->sp_pwdp;
    //3.
    char salt[128]={0};
    int count=0;
    int index=0;
    while(*p)
    {
        salt[index]=*p;
        if(salt[index]=='$')
        {
            count++;
            if(count==3)
            {
                break;
            }
        }
        p++;
        index++;
    }
    //3.对输入的密码进行按照一样的算法，密钥加密
    char* mypasswd=(char*)crypt(password,salt);
    if(strcmp(mypasswd,sp->sp_pwdp)!=0)
    {
        printf("Passwd error\n");
        exit(0);
    }
    //5.成功创建进程，子进程进行新的UID设置，替换为新的默认终端，父进程阻塞
    pid_t pid=fork();
    if(pid==0)
    {
        struct passwd *pw=getpwnam(user);
        assert(pw!=NULL);
        setuid(pw->pw_uid);
        execl(pw->pw_shell,pw->pw_shell,(char*)0);
        perror(pw->pw_shell);
    }
    else
    {

        printf("\n");
        wait(NULL);
    }
    exit(0);
}
