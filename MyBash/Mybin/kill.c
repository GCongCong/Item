# include<stdio.h>
# include<unistd.h>
# include<stdlib.h>
# include<assert.h>
# include<string.h>

int main(int argc,char* argv[])
{
    if(argc<2)
    {
        printf("argc error,too less\n");
        exit(0);
    }
    int sign=15;
    int i=1;
    for(;i<argc;i++)
    {
        if(i==1)
        {
            if(strncmp(argv[1],"-9",2)==0)
            {
                sign=9;
                continue;
            }
            if(strncmp(argv[1],"-stop",5)==0)
            {
                sign=19;
                continue;
            }
        }
        int pid=0;
        sscanf(argv[i],"%d",&pid);
        if(kill(pid,sign)==-1)
        {
            perror("kill error\n");
        }
    }
    exit(0);

}
