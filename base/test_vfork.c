/*
 * A simple test program for vfork function
 * time: 2013-11-08 14:29
 * fork vs vfork:
 * (1).the fork function create a new progress, and exec,vfork��fork������һ���ӽ��̣�
 * ��vfork�����������̵ĵ�ַ�ռ���ȫ���Ƶ��ӽ����У���Ϊ�ӽ��̻���������exec����exit,
 * ����Ҳ�Ͳ����ø����̵ĵ�ַ�ռ�.���ӽ��̵���exec��exit֮ǰ�����ڸ����̵Ŀռ������С�
 * (2). vfork��֤�ӽ��������У���������exec��exit֮�󸸽��̲ſ��ܱ��������С�
 */

#include <sys/types.h>
#include <unistd.h>         // for exit()

#include <stdlib.h>         // for _exit()

#include <stdio.h>

int global_var = 6;

int main()
{
    int local_var = 7;
    pid_t pid;

    printf("before vfork \r\n");
    if ((pid = vfork()) < 0)
    {
        perror("vfork");
        exit(1);
    }
    else if (pid == 0)      /* child process */
    {
        global_var++;
        local_var++;
        _exit(0);
    }

    /* parent process */
    printf("pid = %d, global_var = %d, var = %d\n", getpid(), global_var, local_var);
    exit(0);
}

/*
 * result: pid = 15988, global_var = 7, var = 8
 *
 */

