/*******************************************************************************
**  Copyright (c) 2014, Company
**  All rights reserved.
**	
**  文件说明: libc glob接口使用测试程序
**  创建日期: 2014.02.26
**
**  当前版本：1.0
**  作者：
********************************************************************************/

#include <glob.h>
#include <stdio.h>
#include <unistd.h>

#define PATTERN_1     "*.c"
#define PATTERN_2     "../*.c"

/*******************************************************************************
** 函数名称： Glob
** 函数功能： 按照设定的模式查找全文件名
** 传入参数： 
** 传出参数:  globbuf: 查找结果
** 返回值： 
** 创建作者：
** 创建日期：
** 修改作者： 
** 修改日期： 
********************************************************************************/
void Glob(glob_t *globbuf)
{
    if (globbuf == NULL)
    {
        return;
    }

    int glob_ret, i;
    
    /* 从gl_pathv[2]开始存入glob检索结果 */
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
