<!--
 * @Author: xddcore 1034029664@qq.com
 * @Date: 2023-01-06 20:17:33
 * @LastEditors: Chengsen Dong 1034029664@qq.com
 * @LastEditTime: 2023-01-06 12:51:25
 * @FilePath: /rpi-linux/home/xdd/xddcore/Embedded_Linux/rpi-4b/app/00_makefile/README.md
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
-->
# 关于交叉编译过程
1. 预处理(处理宏定义等)
`aarch64-linux-gnu-gcc -E test.c -o test.i`

2. 编译(.i程序->.s汇编)
`aarch64-linux-gnu-gcc -S test.i -o test.s`

3. 汇编(.s汇编->.o二进制文件)
`aarch64-linux-gnu-gcc -c test.s -o test.o`

4. 链接(.o二进制文件->可执行文件|静态链接)
`aarch64-linux-gnu-gcc test.o -o test --static`