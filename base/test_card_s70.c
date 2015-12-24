#include <stdio.h>

#define FEATURE_SIZE 3112

int main()
{
    int i;
    int iDataBlockNum = 0;
    int iControlBlockNum = 0;
    int iTotalSize = 0;

    for(i = 0; i < 256; i++)
    {
        if (i == 0) {
            printf("Block %d is Manfacturer Block %s %d\r\n", i, __FUNCTION__, __LINE__);
        } else if (i < 128) {
            if (i % 4 == 3) {
                iControlBlockNum++;
                printf("Block %d is Control Block. totalControlBlock is %d. \r\n", i, iControlBlockNum);
            } else {
                iDataBlockNum++;
                iTotalSize += 16;
                printf("Block %d is Data Block. totalDataBlock is %d. totalSize is %d \r\n", i, iDataBlockNum, iTotalSize);
            }
        } else {
            if (i % 16 == 15) {
                iControlBlockNum++;
                printf("Block %d is Control Block. totalControlBlock is %d. \r\n", i, iControlBlockNum);
            } else {
                iDataBlockNum++;
                iTotalSize += 16;
                printf("Block %d is Data Block. totalDataBlock is %d. totalSize is %d \r\n", i, iDataBlockNum, iTotalSize);
            }
        }

        if (iDataBlockNum == 195)
        {
            printf("Block %d is the end block\r\n", i);
        }

        if (iTotalSize >= FEATURE_SIZE)
        {
            printf("################ Block is %d \r\n", i);
            break;
        }
    }
    
    return 0;
}
