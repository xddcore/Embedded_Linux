/*
 * @Author: Chengsen Dong 1034029664@qq.com
 * @Date: 2022-06-11 11:23:49
 * @LastEditors: xddcore 1034029664@qq.com
 * @LastEditTime: 2022-06-11 20:03:43
 * @FilePath: /Embedded_Linux/rpi-4b/driver/01_XGPIO/XGPIO.c
 * @Description: XGPIO 树莓派4b BCM2711 GPIO Linux驱动
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
#define XGPIO_Registerx_Base 0x7e200000 //GPIO寄存器组基地址
/**
 * @description: GPIO寄存器表(BCM2711|RPI-4b)
 * @return {*}
 */
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
}XGPIO_Registerx;

XGPIO_Registerx * pXGPIO_Registerx;//XGPIO_Registerx的指针

/**************************************************************/
static const struct file_operations module_fops={
    .owner = THIS_MODULE,
    .open = NULL,
    .release = NULL,
    .write = NULL,
    .read = NULL,
};

static int __init XGPIO_Init(void)
{
    printk(KERN_INFO "XGPIO: XGPIO Begin Init.\n");
    //注册字符设备
    dev_major = register_chrdev(0, MODULE_NAME, &module_fops);
    if(dev_major < 0)//无法分配设备号
    {
        printk(KERN_ERR "XGPIO: Register Device Fail!\n");
        return -EINVAL;
    }
    printk(KERN_INFO "XGPIO: Register Device Success! Device Major = %d.\n", dev_major);
    //在虚拟内存中申请GPIO寄存器组的空间（如果失败，则返回NULL指针）
    if(!request_mem_region(XGPIO_Registerx_Base, sizeof(XGPIO_Registerx), XGPIO_Registerx_Name))
    {
        //仅检查物理内存区域是否已经被其他应用程序使用，不会进行映射操作。(linux的保护机制)
        //对于rpi官方镜像来说，如果使用默认config编译镜像，则gpio驱动会在机器启动时注册这个区域
        //我们可以将下方return -EINVA；注释，绕过这个检查。（前提是我们再也不会用官方的gpio驱动来干活，否则会打架）。
        //可行性来自于ioremap()的调用没有物理内存空间名检查
        printk(KERN_ERR "XGPIO: Physical Address Region Malloc Check Fail!\n");
        printk(KERN_ERR "XGPIO: XGPIO Will ignore it! Please DON'T Use offical GPIO Driver in feature!\n");
        //return -EINVAL;
    }
    //动态映射GPIO寄存器组
    pXGPIO_Registerx = ioremap(XGPIO_Registerx_Base, sizeof(XGPIO_Registerx));
    printk(KERN_INFO "XGPIO: XGPIO Success!\n");
    return 0;
}

static void __exit XGPIO_Exit(void)
{
    printk(KERN_INFO "XGPIO: XGPIO Begin Release.\n");
    //解除GPIO寄存器组映射
    iounmap(pXGPIO_Registerx);
    //释放虚拟内存中的空间
    release_mem_region(XGPIO_Registerx_Base,sizeof(XGPIO_Registerx));
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