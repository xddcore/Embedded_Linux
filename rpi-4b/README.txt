#uboot+linux kernel+rootfs(buysbox)食用指南
1.在SD卡中创建 FAT32分区和EXT4分区
2.将/udisk_fat32中的所有内容拷贝至SD卡的FAT32分区中
3.将/rootfs_full中的所有内容拷贝至SD卡的EXT4分区中
4.按照/help/use_rootfs.txt中的指引在EXT4分区中创建额外的文件夹供VFS使用
5.将sd卡插入树莓派4b
5.使用USB-TTL模块连接40PIN排线中的串口(在本文中，我们使用的软件串口ttyS0）
6.打开电源，enjoy it!

PS:bps:115200

#linux 驱动开发(目的，使用自己编译的linux内核和modules替换发行版中的内核和modules)
1.使用Raspberry Pi Imager 全自动做一个linux发行版的镜像 至SD卡
2.将/kernel目录下Image文件拷贝至SD卡的FAT32分区中，并FAT32分区中的config.txt文件(将SSBL的内核文件切换为我们使用的，具体代码为:#kernel=vmlinuz
kernel=Image)
3.使用如下命令在内核源码树导出与内核相关的modules(什么是modules，大概就是一些应用程序(比如firefox，亦或者一些外设(比如蓝牙，树莓派4b的蓝牙是挂载在硬件串口ttyAM0上的)的驱动。
缺少了这一个步骤的操作，内核在启动过程中会报错，但可以正常启动。模块的设计使内核源码和一些功能代码分隔开，这样不用每次改功能，就要重新编译内核。
对于32位来说:
sudo env PATH=$PATH make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- INSTALL_MOD_PATH=$(此处改为你要安装的位置，后面再搬到SD卡EXT4分区的/lib/modules/就行) modules_install
对于64位来说:
sudo env PATH=$PATH make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- INSTALL_MOD_PATH=$(此处改为你要安装的位置，后面再搬到SD卡EXT4分区的/lib/modules/就行) modules_install
4.至此保证了服务器上用于交叉编译的内核源码版本和树莓派内部内核版本相同。
5.开始进行驱动开发。

PS:
1. 本教程中所用的内核5.15.44-V8+，发行版 ubuntu22.04，在执行完如上操作后。内核已被成功替换。开机后会提示firefox，snap store等挂载失败。wifi,蓝牙设备无法找到。不过不打算深究了，树莓派上的内核版本只要和交叉编译的内核版本相同即可开始内核开发。
2. 为什么在linux驱动开发中rootfs要用发行版，而不继续之前的busybox。原因:busybox里面东西比较少，像apt之类的都是没有的。产品发布后，用busybox可以极大程度减少成本。不过在驱动开发调试过程中，用发行版还是比较香的。最后也方便快速得到一个.ko(对于驱动开发者来说)。