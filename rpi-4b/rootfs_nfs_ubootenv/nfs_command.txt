#使用nfs挂载rootfs
#nfs服务器搭建:https://blog.csdn.net/qq_37860012/article/details/86717891
#uboot网络设置:https://blog.csdn.net/weixin_45309916/article/details/109178659
#如出现卡死，请测试nfs是否能在本机被正常挂载 (若用服务器 请注意防火墙的问题)

setenv bootargs 'root=/dev/nfs nfsroot=${serverip}:/home/xdd/xddcore/nfsroot,v3,tcp rw ip=${ipaddr}:${serverip}:${gatewayip}:${netmask} ::eth0:on init=/linuxrc 8250.nr_uarts=1 console=ttyS0,115200'

env save
