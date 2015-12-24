/*
 * =====================================================================================
 *
 *       Filename:  sizeof_test.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/23/14 22:19:53
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>

typedef struct
{
    void *start;
    int lenght;
} BUFF_TYPE;

BUFF_TYPE *buf_type;


int main()
{
    printf("sizeof(*buf_type): %d, sizeof(BUFF_TYPE): %d \r\n",
            sizeof(*buf_type), sizeof(BUFF_TYPE));
    printf("sizeof(buf_type): %d \r\n", sizeof(buf_type));

    return 0;
}
