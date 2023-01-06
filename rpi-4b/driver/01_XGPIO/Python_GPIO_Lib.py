'''
Author: Chengsen Dong 1034029664@qq.com
Date: 2023-01-06 21:32:25
LastEditors: Chengsen Dong 1034029664@qq.com
LastEditTime: 2023-01-06 21:33:45
FilePath: /Embedded_Linux/rpi-4b/driver/01_XGPIO/Python_GPIO_Lib.py
Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
'''
import RPi.GPIO as GPIO
import time

GPIO.setmode(GPIO.BCM)
GPIO.setup(2,GPIO.OUT)

while True:
    GPIO.output(2,GPIO.HIGH)
    GPIO.output(2,GPIO.LOW)


GPIO.cleanup()