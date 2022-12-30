<!--
 * @Author: Chengsen Dong 1034029664@qq.com
 * @Date: 2022-06-09 10:03:05
 * @LastEditors: Chengsen Dong 1034029664@qq.com
 * @LastEditTime: 2022-12-30 11:36:59
 * @FilePath: /Embedded_Linux/rpi-4b/driver/README.md
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
-->
# 树莓派4b驱动开发计划

## 使用字符设备驱动
0. Hello World
1. GPIO
2. Interrupt
3. Timer & System Timer
4. UART
5. PWM
6. IIC
7. SPI
8. DMA
9. PCM/I2S Audio

### 验证过程:
Step1: 确保树莓派Linux内核版本为5.15.44-V8+
Step2: `sudo insmod 驱动名.ko`安装驱动
Step3: `dmesg | grep "驱动名"`查看驱动被分配的主设备号
Step4: `sudo mknod /dev/驱动名 c 主设备号 次设备号`在/dev文件夹下创建设备节点
Step5: `sudo ./驱动名_APP` 运行应用层DEMO

# 使用设备驱动框架(如gpiolib,misc类等)