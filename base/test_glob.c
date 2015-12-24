/*******************************************************************************
**  Copyright (c) 2014, Company
**  All rights reserved.
**	
**  �ļ�˵��: libc glob�ӿ�ʹ�ò��Գ���
**  ��������: 2014.02.26
**
**  ��ǰ�汾��1.0
**  ���ߣ�
********************************************************************************/

#include <glob.h>
#include <stdio.h>
#include <unistd.h>

#define PATTERN_1     "*.c"
#define PATTERN_2     "../*.c"

/*******************************************************************************
** �������ƣ� Glob
** �������ܣ� �����趨��ģʽ����ȫ�ļ���
** ��������� 
** ��������:  globbuf: ���ҽ��
** ����ֵ�� 
** �������ߣ�
** �������ڣ�
** �޸����ߣ� 
** �޸����ڣ� 
********************************************************************************/
void Glob(glob_t *globbuf)
{
    if (globbuf == NULL)
    {
        return;
    }

    int glob_ret, i;
    
    /* ��gl_pathv[2]��ʼ����glob������� */
    globbuf->gl_offs = 2;

    glob_ret = glob(PATTERN_1, GLOB_DOOFFS, NULL, globbuf);
    if (glob_ret == GLOB_NOSPACE)
    {
        return;
    }
    
    if (glob_ret == GLOB_ABORTED || glob_ret == GLOB_NOMATCH)
    {
		goto leave;
    }
    
    glob_ret = glob(PATTERN_2, GLOB_DOOFFS | GLOB_APPEND, NULL, globbuf);
    if (glob_ret == GLOB_NOSPACE)
    {
        return;
    }

    if (glob_ret == GLOB_ABORTED || glob_ret == GLOB_NOMATCH)
    {
		goto leave;
    }

    printf("gl_pathc: %d \r\n", globbuf->gl_pathc);
    
    for (i = 0; i < globbuf->gl_pathc; i++)
    {
        printf("gl_pathc[%d]: %s \r\n", i, globbuf->gl_pathv[i]);
    }
    
    globbuf->gl_pathv[0] = "ls";
    globbuf->gl_pathv[1] = "-l";

leave:
    globfree(globbuf);
    return; 
}

int main(int argc, char *argv[])
{
    glob_t globbuf;

    Glob(&globbuf);
    
    execvp("ls", &globbuf.gl_pathv[0]);
    
    return 0;
}
