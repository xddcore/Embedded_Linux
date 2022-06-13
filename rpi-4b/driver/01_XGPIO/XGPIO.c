/*
 * @Author: Chengsen Dong 1034029664@qq.com
 * @Date: 2022-06-11 11:23:49
 * @LastEditors: Chengsen Dong 1034029664@qq.com
 * @LastEditTime: 2022-06-13 21:16:51
 * @FilePath: /Embedded_Linux/rpi-4b/driver/01_XGPIO/XGPIO.c
 * @Description: XGPIO 树莓派4b BCM2711 GPIO Linux驱动
 * 没用任何驱动框架，随便想着写的“野”驱动，
 * 本质就是对底层硬件的访问。关于write和read的str匹配，我留了个结构体数组的实现思路，具体功能没实现。
 * 抛砖引玉交给后来者发挥了。
 * 大概字符串匹配逻辑 就是 用户态 write 字符串“gpio2,<XGPIO_Operation_inout,true>" 实现将gpio2设置为输出（若true为输出的话）
 * 同时支持多个gpio，多个操作同时设置比如将gpio2设置为输出模式，并输出高电平。同时将gpio3设置为输出模式。
 * “gpio2,<XGPIO_Operation_inout,true>,<XGPIO_Operation_setreset,true>gpio3,<XGPIO_Operation_inout,true>”
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
#define XGPIO_Registerx_Name "XGPIO_Registerx"
//arm为内存和io统一编址，下面这个地址为rpi4-bcm2711的映射方式。
//不同的芯片可使用 cat /proc/iomem | grep gpio@ 进行查询
#define XGPIO_Registerx_Base 0xfe200000 //GPIO寄存器组基地址
/**
 * @description: GPIO寄存器表(BCM2711|RPI-4b)
 * @return {*}
 */
//你可以通过以下结构体访问gpio寄存器映射到的虚拟内存。每个成员变量的地址就是实际寄存器的物理地址映射到的虚拟地址。
//结构体提供了一种内存的访问模式，让你的程序根据直观的访问内存
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



//操作id号
#define OPERATION_ID_NUMBER 2
typedef struct{
    const char *operation_name;
    const unsigned int operation_id;
}XGPIO_Operationid_Type[OPERATION_ID_NUMBER];

//GPIO对象可以进行的操作
#define OPERATION_NUMBER 5 //支持的方法数量
//GPIO对象方法类型
typedef struct {
    const char * operation_name;
    int (*func)(unsigned int gpio_id,unsigned int operation_id, unsigned int *result);
}XGPIO_Operation_Type[OPERATION_NUMBER];

#define GPIO_NUMBER 27 //可操作的GPIO数量 BCM2711有57个gpio，不过40PIN引脚引出了29个,除去id_sd&idsc还有27
//GPIO对象类型
typedef struct{
    const char *gpio_name;
    const unsigned int gpio_id;
    XGPIO_Operationid_Type (*pXGPIO_Operationid)[OPERATION_ID_NUMBER];
    XGPIO_Operation_Type (*pXGPIO_Operation)[OPERATION_NUMBER];
} XGPIO_Type[GPIO_NUMBER];



//操作id号实例化
XGPIO_Operationid_Type XGPIO_Operationidx={
    {"true",1},
    {"flase",0},
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
XGPIO_Operation_Type XGPIO_Operationx={
    {"inout",XGPIO_Operation_inout}, 
    {"pullupdown",XGPIO_Operation_pullupdown}, 
    {"setreset",XGPIO_Operation_setreset}, 
    {"pinlevel",XGPIO_Operation_pinlevel}, 
    {"DEBUG",XGPIO_Operation_DEBUG}, 
};

//实例化GPIO对象数组
XGPIO_Type XGPIO_OBJ={
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
/**************************************************************/
//投机取巧之 使用ioctl(cmd,value)实现用户态直接对物理内存的访问
//(可能会导致系统安全系数直线降低，应用程序可利用这个漏洞在物理内存里面随意乱写)
//XGPIO_ioctl 为用户态的应用程序提供直接访问物理地址的接口
//更大的io需求，例如访问显存之类的，还可以使用mmap内存映射实现
unsigned int * pioctl_address;//通过ioctl访问的虚拟内存
int XGPIO_ioctl(unsigned int address, unsigned long value)
{
    //ioremap 将物理地址映射到虚拟地址
    //(在linux中，内核态中的驱动通过将物理地址映射到虚拟地址，实现对io的访问)
    if(!(address>=XGPIO_Registerx_Base && address <=(XGPIO_Registerx_Base+0xf0)))
    {
        //确保安全，只能在GPIO范围内乱写
        return 1;//访问超出GPIO物理地址区域，错误
    }
    pioctl_address = ioremap(address,sizeof(address));
    iowrite32(value,pioctl_address);
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
//XGPIO输入/输出设置方法
int XGPIO_Operation_inout(unsigned int gpio_id,unsigned int operation_id, unsigned int *result)
{
    return 0;
}
//XGPIO上拉/下拉设置方法
int XGPIO_Operation_pullupdown(unsigned int gpio_id,unsigned int operation_id, unsigned int *result)
{
    return 0;
}
//XGPIO电平设置设置方法
int XGPIO_Operation_setreset(unsigned int gpio_id,unsigned int operation_id, unsigned int *result)
{
    return 0;
}
//XGPIO电平读取方法
int XGPIO_Operation_pinlevel(unsigned int gpio_id,unsigned int operation_id, unsigned int *result)
{
    return 0;
}
//XGPIODEBUG(打印所有GPIO寄存器)方法
int XGPIO_Operation_DEBUG(unsigned int gpio_id,unsigned int operation_id, unsigned int *result)
{
    return 0;
}
/**************************************************************/
static const struct file_operations module_fops={
    .owner = THIS_MODULE,
    .open = NULL,
    .release = NULL,
    .write = NULL,
    .read = NULL,
    .unlocked_ioctl = XGPIO_IOCTL,
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
    pXGPIO_Register = ioremap(XGPIO_Registerx_Base, sizeof(XGPIO_Register_Type));
    printk(KERN_INFO "XGPIO: XGPIO Register Sucess!\n");
    return 0;
}

static void __exit XGPIO_Exit(void)
{
    printk(KERN_INFO "XGPIO: XGPIO Begin Release.\n");
    //解除GPIO寄存器组映射
    iounmap(pXGPIO_Register);
    //解除ioctl的访问物理address的虚拟内存映射
    iounmap(pioctl_address);
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