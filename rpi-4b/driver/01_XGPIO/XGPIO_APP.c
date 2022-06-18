/*
 * @Author: Chengsen Dong 1034029664@qq.com
 * @Date: 2022-06-12 10:17:41
 * @LastEditors: Chengsen Dong 1034029664@qq.com
 * @LastEditTime: 2022-06-18 08:18:45
 * @FilePath: /Embedded_Linux/rpi-4b/driver/01_XGPIO/XGPIO_APP.c
 * @Description: XGPIO 树莓派4b BCM2711 GPIO Linux驱动 的APP
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>

unsigned int GPLEV0 = 0;
unsigned int *pGPLEV0 = &GPLEV0;

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
					//
int main(int argc, char* argv[])
{
  unsigned int i=0;
  int fd = open("/dev/XGPIO", O_RDWR);
  if (fd < 0) {
    perror("open device");
    return -1;
  }
  printf("Device File Open Sucess!\n");
  //GPIO2 ioctl访问
  printf("APP send cmd to XGPIO Driver |by ioctl|\n");
  ioctl(fd, XGPIO_Registerx_Base+XGPIO_Registerx_GPFSEL0_Offset, 1<<(3*2));//设置为输出模式
  printf("Set GPIO2 Direction: output\n");
  printf("GPIO2 Will Output 10 Pulse, 0.5duty\n");
  for(i=0;i<10;i++)
  {
    printf("APP send cmd to XGPIO Driver: XGPIO_Registerx_Base+XGPIO_Registerx_GPSET0_Offset,\
     1<<(1*2) |by ioctl|\n");
    ioctl(fd, XGPIO_Registerx_Base+XGPIO_Registerx_GPSET0_Offset, 1<<(1*2));//设置为高电平
    sleep(1);
    printf("APP send cmd to XGPIO Driver: XGPIO_Registerx_Base+XGPIO_Registerx_GPCLR0_Offset,\
     1<<(1*2) |by ioctl|\n");
    ioctl(fd, XGPIO_Registerx_Base+XGPIO_Registerx_GPCLR0_Offset, 1<<(1*2));//设置为低电平
    sleep(1);
  }
  char * cmd1="{gpio2|<inout,true>}";//GPIO2设置为输出模式
  char * cmd2="{gpio2|<setreset,true>}";//GPIO2设置为高电平
  char * cmd3="{gpio2|<setreset,false>}";//GPIO2设置为低电平
  
  char * cmd4="{gpio2|<pinlevel,true>}";//更新GPLEV0寄存器(可以读取GPIO0-31的电平状态)
  
  char * cmd5="{gpio3,gpio4|<inout,false>}";//GPIO3，GPIO4设置为输入模式
  
  char * cmd6="{gpio5|<inout,true>}";//GPIO5设置为输出模式
  char * cmd7="{gpio5|<setreset,true>}";//GPIO5设置为高电平
  char * cmd8="{gpio5|<setreset,false>}";//GPIO5设置为低电平
  printf("\n\nAPP send cmd to XGPIO Driver |by write|:%s\n",cmd1);
  write(fd,cmd1,strlen(cmd1));//GPIO2设置为输出模式
  write(fd,cmd5,strlen(cmd5));//GPIO3，GPIO4设置为输入模式
  write(fd,cmd6,strlen(cmd6));//GPIO5设置为输出模式
  for(i=0;i<10;i++)
  {
    printf("APP send cmd to XGPIO Driver |by write|:%s\n",cmd4);
    write(fd,cmd4,strlen(cmd4));//更新GPLEV0寄存器(可以读取GPIO0-31的电平状态)
    printf("APP read GPLEV0 from XGPIO Driver |by read|\n");
    //read(fd,pGPLEV0,4);//读取一个4个 bytes(char)基类型的长度
    printf("GPLEV0 = 0x%x\n",GPLEV0);

    printf("APP send cmd to XGPIO Driver |by write|:%s\n",cmd2);
    write(fd,cmd2,strlen(cmd2));
    //ioctl(fd, XGPIO_Registerx_Base+0x1c, 1<<(1*2));//设置为高电平
    sleep(1);
    printf("APP send cmd to XGPIO Driver |by write|:%s\n",cmd3);
    write(fd,cmd3,strlen(cmd3));
    //ioctl(fd, XGPIO_Registerx_Base+0x28, 1<<(1*2));//设置为低电平
    sleep(1);
  }

  close(fd);
  return 0;
}
