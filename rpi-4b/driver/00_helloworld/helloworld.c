/*
 * @Author: xddcore 1034029664@qq.com
 * @Date: 2022-06-06 22:53:35
 * @LastEditors: Chengsen Dong 1034029664@qq.com
 * @LastEditTime: 2022-06-09 16:51:08
 * @FilePath: /rpi-linux/rpi_4b_drivers/00_helloworld/helloworld.c
 * @Description: helloword字符驱动设备框架
 * 
 * Copyright (c) 2022 by xddcore 1034029664@qq.com, All Rights Reserved. 
 */

#include <linux/init.h>
#include <linux/module.h>


static int __init hello_init(void)
{
    printk("chardev init sucess!");
    return 0;
}

static void __exit hello_exit(void)
{
    printk("chardev exit sucess!");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Hello wrold module for RPI 4b");
MODULE_ALIAS("HelloWrold");
MODULE_AUTHOR("xdd 1034029664@qq.com");