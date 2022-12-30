<!--
 * @Author: Chengsen Dong 1034029664@qq.com
 * @Date: 2022-06-09 10:03:05
 * @LastEditors: Chengsen Dong 1034029664@qq.com
 * @LastEditTime: 2022-12-30 12:16:09
 * @FilePath: /Embedded_Linux/rpi-4b/help/use_rootfs.md
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
-->
在rootfs目录下执行，以下的空文件夹无法上传至github，但这些空文件夹是vfs的挂载点，比较重要
`mkdir  dev  etc  home  lib   mnt  proc  root   sys  tmp   var -p`