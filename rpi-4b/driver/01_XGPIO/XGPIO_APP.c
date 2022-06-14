/*
 * @Author: Chengsen Dong 1034029664@qq.com
 * @Date: 2022-06-12 10:17:41
 * @LastEditors: Chengsen Dong 1034029664@qq.com
 * @LastEditTime: 2022-06-14 16:43:08
 * @FilePath: /Embedded_Linux/rpi-4b/driver/01_XGPIO/XGPIO_APP.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define XGPIO_Registerx_Base 0xfe200000 //GPIO寄存器组基地址
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

  ioctl(fd, XGPIO_Registerx_Base+0x00, 1<<(3*2));//设置为输出模式
  printf("Set GPIO2 Direction: output\n");
  printf("GPIO2 Will Output 10 Pulse, 0.5duty\n");
  for(i=0;i<10;i++)
  {
    ioctl(fd, XGPIO_Registerx_Base+0x1c, 1<<(1*2));//设置为高电平
    sleep(1);
    ioctl(fd, XGPIO_Registerx_Base+0x28, 1<<(1*2));//设置为低电平
    sleep(1);
  }
  char * cmd1="{gpio2|<inout,true>}";//设置为输出模式
  char * cmd2="{gpio2|<setreset,true>}";//设置为高电平
  char * cmd3="{gpio2|<setreset,false>}";//设置为低电平
  write(fd,cmd1,sizeof(cmd1));
  for(i=0;i<10;i++)
  {
    write(fd,cmd2,sizeof(cmd2));
    //ioctl(fd, XGPIO_Registerx_Base+0x1c, 1<<(1*2));//设置为高电平
    sleep(1);
    write(fd,cmd3,sizeof(cmd3));
    //ioctl(fd, XGPIO_Registerx_Base+0x28, 1<<(1*2));//设置为低电平
    sleep(1);
  }

  close(fd);
  return 0;
}
