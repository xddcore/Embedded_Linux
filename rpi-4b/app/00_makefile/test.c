/*
 * @Author: Chengsen Dong 1034029664@qq.com
 * @Date: 2023-01-06 12:06:26
 * @LastEditors: Chengsen Dong 1034029664@qq.com
 * @LastEditTime: 2023-01-06 12:09:18
 * @FilePath: /Embedded_Linux/rpi-4b/app/00_makefile/test.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define PAGE_SIZE 4096
#define MAX_SIZE 100*PAGE_SIZE

int main(void)
{
    char *buf = (char *)malloc(MAX_SIZE);
    memset(buf,0,MAX_SIZE);
    printf("Buffer address = 0x%p", buf);
    free(buf);
    return 0;
}