# include<stdio.h>
# include<stdlib.h>
# include<unistd.h>
# include<string.h>
# include<assert.h>


int main()
{
    char path[128]={0};
    getcwd(path,127);
    printf("%s\n",path);
}
