/*
 * @Author: Chengsen Dong 1034029664@qq.com
 * @Date: 2023-01-06 21:42:49
 * @LastEditors: Chengsen Dong 1034029664@qq.com
 * @LastEditTime: 2023-01-06 21:43:43
 * @FilePath: /Embedded_Linux/rpi-4b/driver/01_XGPIO/C_WiringPi_GPIO_Lib.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <wiringPi.h>

int main(void) {

wiringPiSetup();

pinMode (8, OUTPUT); //BCM2 -> wiringpi 8

for(;;) {

digitalWrite(8, HIGH);

digitalWrite(8, LOW);

}

}