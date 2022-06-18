/*
 * @Author: Chengsen Dong 1034029664@qq.com
 * @Date: 2022-06-11 11:23:49
 * @LastEditors: Chengsen Dong 1034029664@qq.com
 * @LastEditTime: 2022-06-18 08:46:50
 * @FilePath: /Embedded_Linux/rpi-4b/driver/01_XGPIO/XGPIO.c
 * @Description: XGPIO 树莓派4b BCM2711 GPIO Linux驱动
 * 没用任何驱动框架，随便想着写的“野”驱动，
 * 本质就是对底层硬件的访问。关于write和read的str匹配，我留了个结构体数组的实现思路，具体功能没实现。
 * 抛砖引玉交给后来者发挥了。一想，这个坑我不填的话，应该也没人填了，于是乎把它填完
 * 大概字符串匹配逻辑 就是 用户态 write 字符串“{gpio2|<inout,true>}" 实现将gpio2设置为输出（若true为输出的话）
 * 同时支持多个gpio同时设置比如将gpio2和gpio3设置为输出模式。
 * “{gpio2,gpio3|<inout,true>}”
 * 还支持读取gpio端口电平
 * 先write“{gpio2,gpio3|<pinlevel,true>}”
 * 然后再去read ，会返回一个unsigned int(即GPLEV0寄存器的值)，请注意这个值不是实时的
 * 每次更新值之前，请先write
 * 
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/ioport.h>

#define MODULE_NAME "XGPIO" //模块名
int dev_major;//设备号
/**************************************************************/
/***********************XGPIO 数据结构**************************/
#define XGPIO_Registerx_Name "XGPIO_Registerx"
//arm为内存和io统一编址，下面这个地址为rpi4-bcm2711的映射方式。
//不同的芯片可使用 cat /proc/iomem | grep gpio@ 进行查询
#define XGPIO_Registerx_Base 0xfe200000 //GPIO寄存器组基地址
/******************GPIO功能选择寄存器*******************/
#define XGPIO_Registerx_GPFSEL0_Offset 0x00
#define XGPIO_Registerx_GPFSEL1_Offset 0x04
#define XGPIO_Registerx_GPFSEL2_Offset 0x08
#define XGPIO_Registerx_GPFSEL3_Offset 0x0c
#define XGPIO_Registerx_GPFSEL4_Offset 0x10
#define XGPIO_Registerx_GPFSEL4_Offset 0x14
/******************GPIO输出高低电平寄存器*******************/
#define XGPIO_Registerx_GPSET0_Offset 0x1c
#define XGPIO_Registerx_GPCLR0_Offset 0x28
/******************GPIO电平状态寄存器*******************/
#define XGPIO_Registerx_GPLEV0_Offset 0x34
/******************GPIO上拉下拉寄存器*******************/
#define XGPIO_Registerx_GPIO_PUP_PDN_CNTRL_REG0_Offset 0xe4
#define XGPIO_Registerx_GPIO_PUP_PDN_CNTRL_REG1_Offset 0xe8
/**
 * @description: GPIO寄存器表(BCM2711|RPI-4b)
 * @return {*}
 */
//你可以通过以下结构体访问gpio寄存器映射到的虚拟内存。每个成员变量的地址就是实际寄存器的物理地址映射到的虚拟地址。
//结构体提供了一种内存的访问模式，让你的程序根据直观的访问内存
//不过经过后期研究，发现BCM2711的GPIO相关寄存器并不是线性地址分布的
//具体表现为sizeof下面的类型=0x78,而datasheet中的offset为0xf0
//如果要使用结构体的方式访问，需要定义一些占位的结构体变量
//转念一想，反正都有ioctl提供给应用层直接访问寄存器。
//所以在具体的底层方法中，就直接ioremap了
typedef struct{
    /*GPIO Function Select 0-5*/
    volatile unsigned int GPFSEL0;
    volatile unsigned int GPFSEL1;
    volatile unsigned int GPFSEL2;
    volatile unsigned int GPFSEL3;
    volatile unsigned int GPFSEL4;
    volatile unsigned int GPFSEL5;
    /*GPIO Pin Output Set 0-1*/
    volatile unsigned int GPSET0;
    volatile unsigned int GPSET1;
    /*GPIO Pin Output Clear 0-1*/
    volatile unsigned int GPCLR0;
    volatile unsigned int GPCLR1;
    /*GPIO Pin Level 0-1*/
    volatile unsigned int GPLEV0;
    volatile unsigned int GPLEV1;    
    /*GPIO Pin Event Detect Status 0-1*/
    volatile unsigned int GPEDS0;
    volatile unsigned int GPEDS1;
    /*GPIO Pin Rising Edge Detect Enable 0-1*/
    volatile unsigned int GPREN0;
    volatile unsigned int GPREN1;
    /*GPIO Pin Falling Edge Detect Enable 0-1*/
    volatile unsigned int GPFEN0;
    volatile unsigned int GPFEN1;
    /*GPIO Pin High Detect Enable 0-1*/
    volatile unsigned int GPHEN0;
    volatile unsigned int GPHEN1;
    /*GPIO Pin Low Detect Enable 0-1*/
    volatile unsigned int GPLEN0;
    volatile unsigned int GPLEN1;
    /*GPIO Pin Async. Rising Edge Detect 0-1*/
    volatile unsigned int GPAREN0;
    volatile unsigned int GPAREN1;
    /*GPIO Pin Async. Falling Edge Detect 0-1*/
    volatile unsigned int GPAFEN0;
    volatile unsigned int GPAFEN1;
    /*GPIO Pull-up/Pull-down Register 0-3*/
    volatile unsigned int GPIO_PUP_PDN_CNTRL_REG0;
    volatile unsigned int GPIO_PIP_PDN_CNTRL_REG1;
    volatile unsigned int GPIO_PIP_PDN_CNTRL_REG2;
    volatile unsigned int GPIO_PIP_PDN_CNTRL_REG3;
}XGPIO_Register_Type;

XGPIO_Register_Type *pXGPIO_Register;//pXGPIO_Register的指针
XGPIO_Register_Type XGPIO_Register ={0};//实现对每个GPIO的单独控制，防止影响其他


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
unsigned int operation_result=0;
unsigned int *poperation_result = &operation_result;
/**************************************************************/
/***********************Write 命令解析**************************/
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
    /*printf("驱动收到的命令：%s\n",cmd_str);*/
    /*寻找命令*/
    char *cmd_head = strchr(cmd_str, cmd_head_chr);//命令头部
    char *cmd_end = strchr(cmd_str, cmd_end_chr);//命令尾部
    int cmd_head_index = cmd_head-cmd_str;
    int cmd_end_index = cmd_end-cmd_str;
    /*if (cmd_head&&cmd_end_index)
        printf("命令头部位置: [%d]，尾部位置：[%d]\n",cmd_head_index,cmd_end_index);
    else
        printf("未找到命令\n");*/
    /*寻找对象和属性分隔符*/
    char *obj_attr_seg = strchr(cmd_str, obj_attr_seg_chr);//对象和属性分隔符
    int obj_attr_seg_index = obj_attr_seg-cmd_str;
    /*if (obj_attr_seg)
        printf("对象和属性分隔符，位置: [%d]\n",obj_attr_seg_index);
    else
        printf("未找到对象和属性分隔符\n");*/
    /*寻找对象内部分隔符','*/
    int seg_num=0;//查找到的对象内部分隔符数量
    int seg_index[GPIO_NUMBER]={0};//最多能同时操作27个gpio对象
    //c89不允许在for里面初始化i
    char *i=NULL;
    int j=0;
    int gpio_id_i = 0;
    /*寻找命令中的属性*/
    char *attribute_head = strchr(cmd_str, attribute_head_chr);//命令中的属性头部
    char *attribute_end = strchr(cmd_str, attribute_end_chr);//命令中的属性尾部
    int attribute_head_index = attribute_head-cmd_str;
    int attribute_end_index = attribute_end-cmd_str;
    /*if (cmd_head&&cmd_end_index)
        printf("属性，头部位置: [%d]，尾部位置：[%d]\n",attribute_head_index,attribute_end_index);
    else
        printf("未找到属性\n");*/
    /*寻找属性内部分隔符','*/
    int attri_seg_num=0;//查找到的属性内部分隔符数量
    int attri_seg_index[OPERATION_NUMBER]={0};//最多能同时操作5个gpio对象的属性

    printk(KERN_INFO "receive cmd：%s\n",cmd_str);
    for(i=cmd_head;i<obj_attr_seg;i++)
    {
        char *seg = strchr(i, seg_chr);//命令头部
        if(seg&&seg<=obj_attr_seg) {
            i=seg;//直接从分隔符位置开始下一次查找
            seg_index[seg_num] = seg - cmd_str;
            //printf("对象分隔符，位置: [%d]\n", seg_index[seg_num]);
            seg_num++;
        }
    }
    /*寻找要操作的gpio对象*/
    for(i=cmd_head;i<obj_attr_seg;i++) {//在gpio对象区间内寻找gpio对象
        for(j=0;j<GPIO_NUMBER;j++) {//查找驱动是否支持驱动这个gpio
            if (!strncasecmp(i + 1, XGPIO_OBJ[j].gpio_name, strlen(XGPIO_OBJ[j].gpio_name)))//0找到
            {
                //printf("gpio对象：%s\n", XGPIO_OBJ[j].gpio_name);
                XGPIO_Pin_Set[XGPIO_OBJ[j].gpio_id]=1;//将要操作的gpio写入gpio操作集合
            }
        }
    }
    /*寻找属性内部分隔符','*/
    for(i=attribute_head;i<attribute_end;i++)
    {
        char *seg = strchr(i, seg_chr);//命令头部
        if(seg&&seg<=attribute_end) {
            i=seg;//直接从分隔符位置开始下一次查找
            attri_seg_index[attri_seg_num] = seg - cmd_str;
            //printf("属性分隔符，位置: [%d]\n", attri_seg_index[attri_seg_num]);
            attri_seg_num++;
        }
    }
    /*寻找要操作属性值（也就是方法参数）*/
    for(i=cmd_str+attri_seg_index[0];i<attribute_end;i++) {//在gpio属性区间内寻找gpio操作方法
        for(j=0;j<OPERATION_ID_NUMBER;j++) {//查找驱动是否支持这个操作方法
            if (!strncasecmp(i + 1, XGPIO_Operationidx[j].operation_name, strlen(XGPIO_Operationidx[j].operation_name)))//0找到
            {
                //printf("gpio属性值：%s\n", XGPIO_Operationidx[j].operation_name);
                operation_id=XGPIO_Operationidx[j].operation_id;
            }
        }
    }
    /*寻找要操作属性和属性值(也就是方法名和方法值)*/
    for(i=attribute_head;i<attribute_end;i++) {//在gpio属性区间内寻找gpio操作方法
        for(j=0;j<OPERATION_NUMBER;j++) {//查找驱动是否支持这个操作方法
            if (!strncasecmp(i + 1, XGPIO_Operationx[j].operation_name, strlen(XGPIO_Operationx[j].operation_name)))//0找到
            {
                //printf("gpio方法：%s\n", XGPIO_Operationx[j].operation_name);
                for (gpio_id_i = 0; gpio_id_i < GPIO_NUMBER; gpio_id_i++) {
                    if(XGPIO_Pin_Set[gpio_id_i]==1) {
                        XGPIO_Operationx[j].func(gpio_id_i, operation_id, poperation_result);
                    }
                }
            }
        }
    }
}
/**************************************************************/
/***********************XGPIO 底层方法**************************/
//BUG list:
//2022-06-14 使用结构体映射内存，并用结构体指针访问寄存器的方法未生效
//研究下物理地址和内存地址的映射逻辑
//或改用ioctl里面的iowrite32实现
//XGPIO输入/输出设置方法(1:out,0:input)
int XGPIO_Operation_inout(unsigned int gpio_id,unsigned int operation_id, unsigned int *result)
{
    unsigned int * inout_address;//通过inout_address访问的虚拟内存
    if(gpio_id<=9&&gpio_id>=0)
    {
        //pXGPIO_Register->GPFSEL0=(operation_id<<(gpio_id*3));
        inout_address = ioremap(XGPIO_Registerx_Base+XGPIO_Registerx_GPFSEL0_Offset,4);
        XGPIO_Register.GPFSEL0|=operation_id<<(gpio_id*3);
        iowrite32(XGPIO_Register.GPFSEL0,inout_address);
    }
    else if(gpio_id<=19&&gpio_id>=10)
    {
        //pXGPIO_Register->GPFSEL1=(operation_id<<(gpio_id*3));
        inout_address = ioremap(XGPIO_Registerx_Base+XGPIO_Registerx_GPFSEL1_Offset,4);
        XGPIO_Register.GPFSEL1|=operation_id<<(gpio_id*3);
        iowrite32(XGPIO_Register.GPFSEL1,inout_address);
    }
    else if(gpio_id<=29&&gpio_id>=20)
    {
        //pXGPIO_Register->GPFSEL2=(operation_id<<(gpio_id*3));
        inout_address = ioremap(XGPIO_Registerx_Base+XGPIO_Registerx_GPFSEL2_Offset,4);
        XGPIO_Register.GPFSEL2|=operation_id<<(gpio_id*3);
        iowrite32(XGPIO_Register.GPFSEL2,inout_address);
    }
    /*printk(KERN_INFO "XGPIO: DEBUG-p4-inout-pXGPIO_Register(%p)->GPFSEL0:(%p)|value(0x%x).\n", \
    pXGPIO_Register,&(pXGPIO_Register->GPFSEL0),pXGPIO_Register->GPFSEL0);*/
    printk(KERN_INFO "XGPIO: XGPIO_Operation_inout <gpio%d,operation:%d>!\n",gpio_id,operation_id);
    //解除inout_address的访问物理address的虚拟内存映射
    iounmap(inout_address);
    return 0;
}
//XGPIO上拉/下拉设置方法
//operation_id 1=pullup 0=pulldown
int XGPIO_Operation_pullupdown(unsigned int gpio_id,unsigned int operation_id, unsigned int *result)
{
    unsigned int * pullupdown_address;//通过setreset_address访问的虚拟内存
    if(operation_id==0)operation_id=2;//change to 10(bin)
    if(gpio_id>=0&&gpio_id<=15)//
    {
        pullupdown_address = ioremap(XGPIO_Registerx_Base+XGPIO_Registerx_GPIO_PUP_PDN_CNTRL_REG0_Offset,4);
        XGPIO_Register.GPIO_PIP_PDN_CNTRL_REG0|=operation_id<<(gpio_id*2);
        iowrite32(XGPIO_Register.GPIO_PIP_PDN_CNTRL_REG0,pullupdown_address);
    }
    else
    {
        pullupdown_address = ioremap(XGPIO_Registerx_Base+XGPIO_Registerx_GPIO_PUP_PDN_CNTRL_REG1_Offset,4);
        XGPIO_Register.GPIO_PIP_PDN_CNTRL_REG1|=operation_id<<(gpio_id*2);
        iowrite32(XGPIO_Register.GPIO_PIP_PDN_CNTRL_REG1,pullupdown_address);
    }
    printk(KERN_INFO "XGPIO: XGPIO_Operation_pullupdown <gpio%d,operation:%d>!\n",gpio_id,operation_id);
    //解除inout_address的访问物理address的虚拟内存映射
    iounmap(pullupdown_address);
    return 0;
}
//XGPIO电平设置设置方法
int XGPIO_Operation_setreset(unsigned int gpio_id,unsigned int operation_id, unsigned int *result)
{
    unsigned int * setreset_address;//通过setreset_address访问的虚拟内存
    if(operation_id)//1:high level
    {
        //pXGPIO_Register->GPSET0=(1<<(gpio_id*1));//1:enable
        setreset_address = ioremap(XGPIO_Registerx_Base+XGPIO_Registerx_GPSET0_Offset,4);
        XGPIO_Register.GPSET0|=1<<(gpio_id*1);
        iowrite32(XGPIO_Register.GPSET0,setreset_address);
    }
    else
    {
        //pXGPIO_Register->GPCLR0=(1<<(gpio_id*1));//1:enable
        setreset_address = ioremap(XGPIO_Registerx_Base+XGPIO_Registerx_GPCLR0_Offset,4);
        XGPIO_Register.GPCLR0|=1<<(gpio_id*1);
        iowrite32(XGPIO_Register.GPCLR0,setreset_address);
    }
    printk(KERN_INFO "XGPIO: XGPIO_Operation_setreset <gpio%d,operation:%d>!\n",gpio_id,operation_id);
    /*printk(KERN_INFO "XGPIO: DEBUG-p5-setreset-pXGPIO_Register(%p)->GPSET0:(%p)|value(0x%x).\n", \
    pXGPIO_Register,&(pXGPIO_Register->GPSET0),pXGPIO_Register->GPSET0);
    printk(KERN_INFO "XGPIO: DEBUG-p6-setreset-pXGPIO_Register(%p)->GPCLR0:(%p)|value(0x%x).\n", \
    pXGPIO_Register,&(pXGPIO_Register->GPCLR0),pXGPIO_Register->GPCLR0);*/
    //解除inout_address的访问物理address的虚拟内存映射
    iounmap(setreset_address);
    return 0;
}
//XGPIO电平读取方法
int XGPIO_Operation_pinlevel(unsigned int gpio_id,unsigned int operation_id, unsigned int *result)
{
    unsigned int *pinlevel_address=NULL;
    pinlevel_address = ioremap(XGPIO_Registerx_Base+XGPIO_Registerx_GPLEV0_Offset,4);
    *result=ioread32(pinlevel_address);
    printk(KERN_INFO "XGPIO: XGPIO_Operation_pinlevel <gpio%d,operation:%d>!\n \
    Please use read(len=1) to get GPLEV0 register value\n",gpio_id,operation_id);
    return 0;
}
//XGPIODEBUG(打印所有GPIO寄存器)方法
int XGPIO_Operation_DEBUG(unsigned int gpio_id,unsigned int operation_id, unsigned int *result)
{
    printk(KERN_INFO "XGPIO: XGPIO_Operation_DEBUG <gpio%d,operation:%d>!\n",gpio_id,operation_id);
    return 0;
}
/**************************************************************/
/***********************Linux 文件相关**************************/
//投机取巧之 使用ioctl(cmd,value)实现用户态直接对物理内存的访问
//(可能会导致系统安全系数直线降低，应用程序可利用这个漏洞在物理内存里面随意乱写)
//XGPIO_ioctl 为用户态的应用程序提供直接访问物理地址的接口
//更大的io需求，例如访问显存之类的，还可以使用mmap内存映射实现
int XGPIO_ioctl(unsigned int address, unsigned long value)
{
    unsigned int * pioctl_address;//通过ioctl访问的虚拟内存
    //ioremap 将物理地址映射到虚拟地址
    //(在linux中，内核态中的驱动通过将物理地址映射到虚拟地址，实现对io的访问)
    if(!(address>=XGPIO_Registerx_Base && address <=(XGPIO_Registerx_Base+0xf0)))
    {
        //确保安全，只能在GPIO范围内乱写
        return 1;//访问超出GPIO物理地址区域，错误
    }
    pioctl_address = ioremap(address,sizeof(address));
    iowrite32(value,pioctl_address);
    //解除ioctl的访问物理address的虚拟内存映射
    iounmap(pioctl_address);
    return 0;
}
long XGPIO_IOCTL(struct file * filp, unsigned int address, unsigned long value)
{
    int status=0;
    status = XGPIO_ioctl(address, value);
    //执行XGPIO_ioctl失败
    if(status){printk(KERN_ERR "XGPIO: Invail Physical Address, You only allow to access gpio address region!\n");return -EFAULT;}
    return 0;
}

// 向设备文件写入命令后，回来到这个处理函数
ssize_t XGPIO_Write(struct file* filp, const char __user* buf, size_t len, loff_t* off)
{
  char cmd[250]={0};
  char *pcmd = cmd;
  int rc = 0;
  printk(KERN_INFO "XGPIO: DEBUG-p1.\n");
  rc = copy_from_user(cmd, buf, len);
  if (rc < 0) {
    return rc;
  }
  //*off = 0; // 每次控制之后，文件索引都回到开始
  printk(KERN_INFO "XGPIO: DEBUG-p2.\n");
  write_cmd_handler(pcmd);//解析命令并执行(若命令无效，不会报错，也不会影响底层寄存器)
  printk(KERN_INFO "XGPIO: DEBUG-p3.\n");
  return 0;
}
// 向设备文件读取命令后，回来到这个处理函数
ssize_t XGPIO_Read(struct file* filp, char __user* buf, size_t len, loff_t* off)
{
  int rc = 0;
  printk(KERN_INFO "XGPIO: DEBUG-p7.\n");
  rc = copy_to_user(buf, (char *)poperation_result, len);//无论应用想读多长，只能读4bytes,应用层的len应该=1
  if (rc < 0) {
    return rc;
  }
  //*off = 0; // 每次控制之后，文件索引都回到开始
  printk(KERN_INFO "XGPIO: DEBUG-p8.\n");
  printk(KERN_INFO "XGPIO: DEBUG-p9.\n");
  return 0;
}
/**************************************************************/
static const struct file_operations module_fops={
    .owner = THIS_MODULE,
    .open = NULL,
    .release = NULL,
    .write = XGPIO_Write,
    .read = NULL,
    .unlocked_ioctl = XGPIO_IOCTL,//XGPIO_IOCTL, echo "xxx" > 会来到这里
};

static int __init XGPIO_Init(void)
{
    printk(KERN_INFO "XGPIO: XGPIO Begin Init.\n");
    //注册字符设备
    dev_major = register_chrdev(0, MODULE_NAME, &module_fops);
    if(dev_major < 0)//无法分配设备号
    {
        printk(KERN_ERR "XGPIO: Register Device Fail!\n");
        return -EFAULT;
    }
    printk(KERN_INFO "XGPIO: Register Device Success! Device Major = %d.\n", dev_major);
    //在虚拟内存中申请GPIO寄存器组的空间（如果失败，则返回NULL指针）
    if(!request_mem_region(XGPIO_Registerx_Base, sizeof(XGPIO_Register_Type), XGPIO_Registerx_Name))
    {
        //仅检查物理内存区域是否已经被其他应用程序使用，不会进行映射操作。(linux的保护机制)
        //对于rpi官方镜像来说，如果使用默认config编译镜像，则gpio驱动会在机器启动时注册这个区域
        //我们可以将下方return -EFAULT；注释，绕过这个检查。（前提是我们再也不会用官方的gpio驱动来干活，否则会打架）。
        //可行性来自于ioremap()的调用没有物理内存空间名检查
        printk(KERN_ERR "XGPIO: Physical Address Region Malloc Check Fail!\n");
        printk(KERN_ERR "XGPIO: XGPIO Will ignore it! Please DON'T Use offical GPIO Driver in future!\n");
        //return -EFAULT;
    }
    //动态映射GPIO寄存器组(物理地址->虚拟地址)
    //pXGPIO_Register = ioremap(XGPIO_Registerx_Base, sizeof(XGPIO_Register_Type));
    printk(KERN_INFO "XGPIO: XGPIO Register Sucess!\n");
    return 0;
}

static void __exit XGPIO_Exit(void)
{
    printk(KERN_INFO "XGPIO: XGPIO Begin Release.\n");
    //解除GPIO寄存器组映射
    //iounmap(pXGPIO_Register);
    //释放虚拟内存中的空间
    release_mem_region(XGPIO_Registerx_Base,sizeof(XGPIO_Register_Type));
    //注销字符设备
    unregister_chrdev(dev_major,MODULE_NAME);
    printk(KERN_INFO "XGPIO: XGPIO Release Sucess!\n");
}

module_init(XGPIO_Init);
module_exit(XGPIO_Exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("XGPIO module for RPI-4b|BCM2711");
MODULE_ALIAS("XGPIO");
MODULE_AUTHOR("xddcore 1034029664@qq.com");