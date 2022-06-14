#include <stdio.h>
#include "string.h"

#define GPIO_NUMBER 3 //可操作的GPIO数量 BCM2711有57个gpio，不过40PIN引脚引出了29个,除去id_sd&idsc还有27
#define OPERATION_NUMBER 5 //支持的方法数量

int main(void)
{
    char *gpio_name[GPIO_NUMBER]={"gpio2","gpio3","gpio4"};
    char *operation_name[1]={"inout"};
    char *operation_id[2]={"true","false"};
    printf("命令解析以及运行思路:先解析出对象属性和属性值，再去解析要操作的对象，最后调用结构体数组结构体数组函数指针，最终完成对gpio对象的控制\n");
    //计算字符w在字符串string中的位置
    char* string = "{gpio2,gpio3|<inout,true>}"; //write进来的命令

    
    const char cmd_head_chr = '{';
    const char cmd_end_chr = '}';
    const char attribute_head_chr = '<';
    const char attribute_end_chr = '>';
    const char obj_attr_seg_chr = '|';//对象和属性之间的分隔符
    const char seg_chr = ',';//内部分隔符

    printf("驱动收到的命令：%s\n",string);
    /*寻找命令*/
    char *cmd_head = strchr(string, cmd_head_chr);//命令头部
    char *cmd_end = strchr(string, cmd_end_chr);//命令尾部
    int cmd_head_index = cmd_head-string;
    int cmd_end_index = cmd_end-string;
    if (cmd_head&&cmd_end_index)
        printf("命令头部位置: [%d]，尾部位置：[%d]\n",cmd_head_index,cmd_end_index);
    else
        printf("未找到命令\n");

    /*寻找对象和属性分隔符*/
    char *obj_attr_seg = strchr(string, obj_attr_seg_chr);//对象和属性分隔符
    int obj_attr_seg_index = obj_attr_seg-string;
    if (obj_attr_seg)
        printf("对象和属性分隔符，位置: [%d]\n",obj_attr_seg_index);
    else
        printf("未找到对象和属性分隔符\n");


    /*寻找对象内部分隔符','*/
    int seg_num=0;//查找到的对象内部分隔符数量
    int seg_index[GPIO_NUMBER]={0};//最多能同时操作27个gpio对象
    for(char *i=cmd_head;i<obj_attr_seg;i++)
    {
        char *seg = strchr(i, seg_chr);//命令头部
        if(seg&&seg<=obj_attr_seg) {
            i=seg;//直接从分隔符位置开始下一次查找
            seg_index[seg_num] = seg - string;
            printf("对象分隔符，位置: [%d]\n", seg_index[seg_num]);
            seg_num++;
        }
    }

    /*寻找要操作的gpio对象*/
    for(char *i=cmd_head;i<obj_attr_seg;i++) {//在gpio对象区间内寻找gpio对象
        for(int j=0;j<GPIO_NUMBER;j++) {//查找驱动是否支持驱动这个gpio
            if (!strncasecmp(i + 1, gpio_name[j], strlen(gpio_name[j])))//0找到
            {
                printf("gpio对象：%s\n", gpio_name[j]);
            }
        }
    }
    /*寻找命令中的属性*/
    char *attribute_head = strchr(string, attribute_head_chr);//命令中的属性头部
    char *attribute_end = strchr(string, attribute_end_chr);//命令中的属性尾部
    int attribute_head_index = attribute_head-string;
    int attribute_end_index = attribute_end-string;
    if (cmd_head&&cmd_end_index)
        printf("属性，头部位置: [%d]，尾部位置：[%d]\n",attribute_head_index,attribute_end_index);
    else
        printf("未找到属性\n");

    /*寻找属性内部分隔符','*/
    int attri_seg_num=0;//查找到的属性内部分隔符数量
    int attri_seg_index[OPERATION_NUMBER]={0};//最多能同时操作5个gpio对象的属性
    for(char *i=attribute_head;i<attribute_end;i++)
    {
        char *seg = strchr(i, seg_chr);//命令头部
        if(seg&&seg<=attribute_end) {
            i=seg;//直接从分隔符位置开始下一次查找
            attri_seg_index[attri_seg_num] = seg - string;
            printf("属性分隔符，位置: [%d]\n", attri_seg_index[attri_seg_num]);
            attri_seg_num++;
        }
    }

    /*寻找要操作属性和属性值（也就是方法名和方法参数）*/
    for(char *i=attribute_head;i<attribute_end;i++) {//在gpio属性区间内寻找gpio操作方法
        for(int j=0;j<1;j++) {//查找驱动是否支持这个操作方法
            if (!strncasecmp(i + 1, operation_name[j], strlen(operation_name[j])))//0找到
            {
                printf("gpio方法：%s\n", operation_name[j]);
            }
        }
    }

    /*寻找要操作属性和属性值（也就是方法名和方法参数）*/
    for(char *i=string+attri_seg_index[0];i<attribute_end;i++) {//在gpio属性区间内寻找gpio操作方法
        for(int j=0;j<2;j++) {//查找驱动是否支持这个操作方法
            if (!strncasecmp(i + 1, operation_id[j], strlen(operation_id[j])))//0找到
            {
                printf("gpio属性值：%s\n", operation_id[j]);
            }
        }
    }

}
