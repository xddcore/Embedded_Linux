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
  //GPIO2
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
  close(fd);
  printf("Device File Close Sucess!\n");
  return 0;
}
