<!--
 * @Author: Chengsen Dong 1034029664@qq.com
 * @Date: 2023-01-06 20:29:54
 * @LastEditors: Chengsen Dong 1034029664@qq.com
 * @LastEditTime: 2023-01-06 22:07:13
 * @FilePath: /Embedded_Linux/rpi-4b/driver/01_XGPIO/README.md
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
-->
# XGPIO

引脚连接请参考:`Embedded_Linux/rpi-4b/datasheet/rpi_SCH_4b_4p0_reduced.pdf`


1. `XGPIO.c` 字符驱动源码
2. `XGPIO.ko` 字符驱动
3. `XGPIO_APP.c` 应用层程序源码
4. `XGPIO_APP` 应用层程序
5. `XGPIO_APP_Performance.c` 应用层程序（性能测试）源码
6. `XGPIO_APP_Performance` 应用层程序（性能测试）

## 性能测试结果

|  库  | 最大GPIO翻转频率  |
|  ----  | ----  |
| XGPIO  | 147kHz |
| python库`RPi.GPIO` | 900kHz |
| C库`wiringPi` | 34.6Mhz(已失真为正弦波) |


> Note: wiringPi:https://github.com/WiringPi/WiringPi/releases/tag/2.61-1

