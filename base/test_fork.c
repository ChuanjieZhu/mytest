/*
 * A simple test program for fork function
 * fork: ����0��ʾ�ӽ���, >0��ʾ�����̣�ֵΪ�ӽ���id���ӽ��̴������ø��������ݿռ䣬
 * �ѣ�ջ�ĸ��������ӽ��̲���������Щ�洢�ռ䣬���ӽ��̹������ĶΡ�
 */

#include <unistd.h>     /* for fork function */

#include <stdlib.h>     /* for exit function */
#include <stdio.h>      /* for perror function */    

static int global_var = 9;

int main()
{
    pid_t pid;
    int local_var = 9;

    if ((pid = fork()) < 0) 
    {
        perror("fork");
        exit(1);
    } 
    else if (pid == 0)  /* child process */
    {
        global_var++;
        local_var++;
    }
    else        /* parent process */
    {
        sleep(2);
    }

    printf("pid = %d, global_var = %d, local_var = %d\n", getpid(), global_var, local_var);

    return 0;
}

