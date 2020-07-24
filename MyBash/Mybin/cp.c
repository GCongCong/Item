# include<stdio.h>
# include<stdlib.h>
# include<string.h>
# include<sys/stat.h>
# include<fcntl.h>
# include<assert.h>

int main(int argc,char* argv[])
{
    if(argc<3)
    {
        printf("Argument Less\n");
    }

    int fd=open(argv[1],O_RDONLY);
    if(fd==-1)
    {
        perror("open error\n");
        exit(0);
    }
    char path[128]={0};
    strcpy(path,argv[2]);

    struct stat st;
    int n=stat(argv[2],&st);
    if(n!=-1 && S_ISDIR(st.st_mode))
    {
        char* p=argv[1]+strlen(argv[1]);
        while(p!=argv[1] && *p!='/')
        {
            p--;
        }
        strcat(path,"/");
        strcat(path,p);
    }

    int fw=open(path,O_CREAT|O_WRONLY|O_TRUNC,0664);
    if(fw==-1)
    {
        perror("open2 error\n");
        exit(0);
    }

    while(1)
    {
        char buff[128]={0};
        int num=read(fd,buff,127);
        if(num<=0)
        {
            break;
        }
        int res=write(fw,buff,num);
        if(res<=0)
        {
            break;
        }
    }

    close(fd);
    close(fw);
}
