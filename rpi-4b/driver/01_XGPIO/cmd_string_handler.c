#include <stdio.h>
#include "string.h"

//操作id号
#define OPERATION_ID_NUMBER 2
typedef struct{
    const char *operation_name;
    const unsigned int operation_id;
}XGPIO_Operationid_Type;

//GPIO对象可以进行的操作
#define OPERATION_NUMBER 5 //支持的方法数量
//GPIO对象方法类型
typedef struct {
    const char * operation_name;
    int (*func)(unsigned int gpio_id,unsigned int operation_id, unsigned int *result);
}XGPIO_Operation_Type;

#define GPIO_NUMBER 26 //可操作的GPIO数量 BCM2711有57个gpio，不过40PIN引脚引出了29个,除去id_sd&idsc还有27
//GPIO对象类型
typedef struct{
    const char *gpio_name;
    const unsigned int gpio_id;
    XGPIO_Operationid_Type *pXGPIO_Operationid; //编译器对结构体数组指针的识别有bug，所以此处用结构体指针，也能实现功能
    XGPIO_Operation_Type *pXGPIO_Operation;
} XGPIO_Type;


//操作id号实例化
XGPIO_Operationid_Type XGPIO_Operationidx[OPERATION_ID_NUMBER]={
        {"true",1},
        {"false",0},
};

//下面的前3个对象方法，操作逻辑上来说，都是2值(要么开要么关)
//第四个读取电平高低直接传个result指针就行，然后再从里面拿相应端口的电平结果
//第五个debug设计为以四字节 打印所有gpio有关寄存器
//方法声明
int XGPIO_Operation_inout(unsigned int gpio_id,unsigned int operation_id, unsigned int *result);
int XGPIO_Operation_pullupdown(unsigned int gpio_id,unsigned int operation_id, unsigned int *result);
int XGPIO_Operation_setreset(unsigned int gpio_id,unsigned int operation_id, unsigned int *result);
int XGPIO_Operation_pinlevel(unsigned int gpio_id,unsigned int operation_id, unsigned int *result);
int XGPIO_Operation_DEBUG(unsigned int gpio_id,unsigned int operation_id, unsigned int *result);

//实例化方法
XGPIO_Operation_Type XGPIO_Operationx[OPERATION_NUMBER]={
        {"inout",XGPIO_Operation_inout},
        {"pullupdown",XGPIO_Operation_pullupdown},
        {"setreset",XGPIO_Operation_setreset},
        {"pinlevel",XGPIO_Operation_pinlevel},
        {"DEBUG",XGPIO_Operation_DEBUG},
};

//实例化GPIO对象数组
XGPIO_Type XGPIO_OBJ[GPIO_NUMBER]={
        {"gpio2", 2, XGPIO_Operationidx, XGPIO_Operationx},
        {"gpio3", 3, XGPIO_Operationidx, XGPIO_Operationx},
        {"gpio4", 4, XGPIO_Operationidx, XGPIO_Operationx},
        {"gpio14", 14, XGPIO_Operationidx, XGPIO_Operationx},
        {"gpio15", 15, XGPIO_Operationidx, XGPIO_Operationx},
        {"gpio18", 18, XGPIO_Operationidx, XGPIO_Operationx},
        {"gpio17", 17, XGPIO_Operationidx, XGPIO_Operationx},
        {"gpio27", 27, XGPIO_Operationidx, XGPIO_Operationx},
        {"gpio22", 22, XGPIO_Operationidx, XGPIO_Operationx},
        {"gpio23", 23, XGPIO_Operationidx, XGPIO_Operationx},
        {"gpio24", 24, XGPIO_Operationidx, XGPIO_Operationx},
        {"gpio10", 10, XGPIO_Operationidx, XGPIO_Operationx},
        {"gpio9", 9, XGPIO_Operationidx, XGPIO_Operationx},
        {"gpio11", 11, XGPIO_Operationidx, XGPIO_Operationx},
        {"gpio25", 25,  XGPIO_Operationidx, XGPIO_Operationx},
        {"gpio8", 8,  XGPIO_Operationidx, XGPIO_Operationx},
        {"gpio7", 7,  XGPIO_Operationidx, XGPIO_Operationx},
        //{"id_sd", 0},
        //{"id_sc", 0},
        {"gpio5", 5, XGPIO_Operationidx, XGPIO_Operationx},
        {"gpio6", 6, XGPIO_Operationidx, XGPIO_Operationx},
        {"gpio12", 12, XGPIO_Operationidx, XGPIO_Operationx},
        {"gpio13", 13, XGPIO_Operationidx, XGPIO_Operationx},
        {"gpio19", 19, XGPIO_Operationidx, XGPIO_Operationx},
        {"gpio26", 26, XGPIO_Operationidx, XGPIO_Operationx},
        {"gpio16", 16, XGPIO_Operationidx, XGPIO_Operationx},
        {"gpio20", 20, XGPIO_Operationidx, XGPIO_Operationx},
        {"gpio21", 21, XGPIO_Operationidx, XGPIO_Operationx},
};

//XGPIO输入/输出设置方法
int XGPIO_Operation_inout(unsigned int gpio_id,unsigned int operation_id, unsigned int *result)
{
    printf("XGPIO输入/输出设置方法:gpio_id:%d | operation_id :%d\n",gpio_id,operation_id);
    return 0;
}
//XGPIO上拉/下拉设置方法
int XGPIO_Operation_pullupdown(unsigned int gpio_id,unsigned int operation_id, unsigned int *result)
{
    printf("XGPIO上拉/下拉设置方法");
    return 0;
}
//XGPIO电平设置设置方法
int XGPIO_Operation_setreset(unsigned int gpio_id,unsigned int operation_id, unsigned int *result)
{
    printf("XGPIO电平设置设置方法");
    return 0;
}
//XGPIO电平读取方法
int XGPIO_Operation_pinlevel(unsigned int gpio_id,unsigned int operation_id, unsigned int *result)
{
    printf("XGPIO电平读取方法");
    return 0;
}
//XGPIODEBUG(打印所有GPIO寄存器)方法
int XGPIO_Operation_DEBUG(unsigned int gpio_id,unsigned int operation_id, unsigned int *result)
{
    printf("XGPIODEBUG(打印所有GPIO寄存器)方法");
    return 0;
}



/*Write命令解析函数*/
void write_cmd_handler(char * cmd_str)
{
    const char cmd_head_chr = '{';
    const char cmd_end_chr = '}';
    const char attribute_head_chr = '<';
    const char attribute_end_chr = '>';
    const char obj_attr_seg_chr = '|';//对象和属性之间的分隔符
    const char seg_chr = ',';//内部分隔符

    unsigned int XGPIO_Pin_Set[GPIO_NUMBER]={0};//要操作的GPIO(索引表示)集合，如果gpio1要操作，则XGPIO_Pin_Set[1]=1;
    unsigned int operation_id=0;//命令中的属性值
    printf("驱动收到的命令：%s\n",cmd_str);
    /*寻找命令*/
    char *cmd_head = strchr(cmd_str, cmd_head_chr);//命令头部
    char *cmd_end = strchr(cmd_str, cmd_end_chr);//命令尾部
    int cmd_head_index = cmd_head-cmd_str;
    int cmd_end_index = cmd_end-cmd_str;
    if (cmd_head&&cmd_end_index)
        printf("命令头部位置: [%d]，尾部位置：[%d]\n",cmd_head_index,cmd_end_index);
    else
        printf("未找到命令\n");

    /*寻找对象和属性分隔符*/
    char *obj_attr_seg = strchr(cmd_str, obj_attr_seg_chr);//对象和属性分隔符
    int obj_attr_seg_index = obj_attr_seg-cmd_str;
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
            seg_index[seg_num] = seg - cmd_str;
            printf("对象分隔符，位置: [%d]\n", seg_index[seg_num]);
            seg_num++;
        }
    }
    /*寻找要操作的gpio对象*/
    for(char *i=cmd_head;i<obj_attr_seg;i++) {//在gpio对象区间内寻找gpio对象
        for(int j=0;j<GPIO_NUMBER;j++) {//查找驱动是否支持驱动这个gpio
            if (!strncasecmp(i + 1, XGPIO_OBJ[j].gpio_name, strlen(XGPIO_OBJ[j].gpio_name)))//0找到
            {
                printf("gpio对象：%s\n", XGPIO_OBJ[j].gpio_name);
                XGPIO_Pin_Set[XGPIO_OBJ[j].gpio_id]=1;//将要操作的gpio写入gpio操作集合
            }
        }
    }
    /*寻找命令中的属性*/
    char *attribute_head = strchr(cmd_str, attribute_head_chr);//命令中的属性头部
    char *attribute_end = strchr(cmd_str, attribute_end_chr);//命令中的属性尾部
    int attribute_head_index = attribute_head-cmd_str;
    int attribute_end_index = attribute_end-cmd_str;
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
            attri_seg_index[attri_seg_num] = seg - cmd_str;
            printf("属性分隔符，位置: [%d]\n", attri_seg_index[attri_seg_num]);
            attri_seg_num++;
        }
    }

    /*寻找要操作属性值（也就是方法参数）*/
    for(char *i=cmd_str+attri_seg_index[0];i<attribute_end;i++) {//在gpio属性区间内寻找gpio操作方法
        for(int j=0;j<OPERATION_ID_NUMBER;j++) {//查找驱动是否支持这个操作方法
            if (!strncasecmp(i + 1, XGPIO_Operationidx[j].operation_name, strlen(XGPIO_Operationidx[j].operation_name)))//0找到
            {
                printf("gpio属性值：%s\n", XGPIO_Operationidx[j].operation_name);
                operation_id=XGPIO_Operationidx[j].operation_id;
            }
        }
    }

    /*寻找要操作属性和属性值(也就是方法名和方法值)*/
    for(char *i=attribute_head;i<attribute_end;i++) {//在gpio属性区间内寻找gpio操作方法
        for(int j=0;j<OPERATION_NUMBER;j++) {//查找驱动是否支持这个操作方法
            if (!strncasecmp(i + 1, XGPIO_Operationx[j].operation_name, strlen(XGPIO_Operationx[j].operation_name)))//0找到
            {
                printf("gpio方法：%s\n", XGPIO_Operationx[j].operation_name);
                for (int gpio_id = 0; gpio_id < GPIO_NUMBER; ++gpio_id) {
                    if(XGPIO_Pin_Set[gpio_id]==1) {
                        XGPIO_Operationx[j].func(gpio_id, operation_id, NULL);
                    }
                }
            }
        }
    }
}


int main(void)
{
    char *gpio_name[GPIO_NUMBER]={"gpio2","gpio3","gpio4"};
    char *operation_name[1]={"inout"};
    char *operation_id[2]={"true","false"};
    
    printf("命令解析以及运行思路:先解析出对象属性和属性值，再去解析要操作的对象，最后调用结构体数组结构体数组函数指针，最终完成对gpio对象的控制\n");
    //计算字符w在字符串string中的位置
    char* cmd_str = "{gpio2,gpio2,gpio4,gpio5|<inout,true>}"; //write进来的命令
    write_cmd_handler(cmd_str);

}
