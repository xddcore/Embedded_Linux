#使用makefile快速编译模块驱动,并将编译结果cp到github仓库
MODULE_NAME=helloworld

MODULE_DIR=$(shell basename `pwd`) #获取当前驱动文件夹名

obj-m:=$(MODULE_NAME).o

KERNEL_DIR=/home/xdd/xddcore/rpi-linux/rpi-linux

CROSS=ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-

GITHUB_DIR=/home/xdd/xddcore/Embedded_Linux/rpi-4b/driver

GITHUB_ROOTDIR=/home/xdd/xddcore/Embedded_Linux

all:
	make $(CROSS) -C $(KERNEL_DIR) M=`pwd` modules
	echo $(MODULE_DIR)
	cd $(GITHUB_DIR) && mkdir -p $(MODULE_DIR) 
	cp `pwd`/$(MODULE_NAME).ko  $(GITHUB_DIR)/$(MODULE_DIR)
	cp `pwd`/$(MODULE_NAME).c  $(GITHUB_DIR)/$(MODULE_DIR)
	cp `pwd`/$(MODULE_NAME).o  $(GITHUB_DIR)/$(MODULE_DIR)
	cp `pwd`/Makefile  $(GITHUB_DIR)/$(MODULE_DIR)

cpfromgit:
	cp $(GITHUB_DIR)/$(strip $(MODULE_DIR))/$(MODULE_NAME).c `pwd`

gitpull:
	cd $(GITHUB_ROOTDIR) && git pull

clean:
	make $(CROSS) -C $(KERNEL_DIR) M=`pwd` clean