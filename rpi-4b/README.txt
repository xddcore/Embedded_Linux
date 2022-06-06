#食用指南
1.在SD卡中创建 FAT32分区和EXT4分区
2.将/udisk_fat32中的所有内容拷贝至SD卡的FAT32分区中
3.将/rootfs_full中的所有内容拷贝至SD卡的EXT4分区中
4.按照/help/use_rootfs.txt中的指引在EXT4分区中创建额外的文件夹供VFS使用
5.将sd卡插入树莓派4b
5.使用USB-TTL模块连接40PIN排线中的串口(在本文中，我们使用的软件串口ttyS0）
6.打开电源，enjoy it!

PS:bps:115200