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