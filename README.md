# 从零实现简易播放器
    作者:史正
    邮箱:shizheng163@126.com
    如有错误还请及时指正
    如果有错误的描述给您带来不便还请见谅
    如需交流请发送邮件,欢迎联系

-   我的csdn    : **[https://blog.csdn.net/shizheng163](https://blog.csdn.net/shizheng163)**<br>
-   我的github  : **[https://github.com/shizheng163](https://github.com/shizheng163)**

**目录**
- [从零实现简易播放器](#%E4%BB%8E%E9%9B%B6%E5%AE%9E%E7%8E%B0%E7%AE%80%E6%98%93%E6%92%AD%E6%94%BE%E5%99%A8)
- [简述](#%E7%AE%80%E8%BF%B0)
- [变更记录](#%E5%8F%98%E6%9B%B4%E8%AE%B0%E5%BD%95)
- [0.音视频基础简述](#0%E9%9F%B3%E8%A7%86%E9%A2%91%E5%9F%BA%E7%A1%80%E7%AE%80%E8%BF%B0)
- [1. 模拟视频播放](#1-%E6%A8%A1%E6%8B%9F%E8%A7%86%E9%A2%91%E6%92%AD%E6%94%BE)
- [2. opengl渲染yuv图像](#2-opengl%E6%B8%B2%E6%9F%93yuv%E5%9B%BE%E5%83%8F)
***

# 简述
    从零实现简易播放器Demo, 加深对音视频编程的了解。

github地址:[https://github.com/shizheng163/MyMediaPlayer](https://github.com/shizheng163/MyMediaPlayer)

注意:
-   `前期关于播放器的开发环境都是Windows + QT5.7.1 MSVC2015`

# 变更记录
-   [变更记录](./doc/ChangeLog.md)

# 0.音视频基础简述

-   [音视频基本概念](./doc/0.音视频基本概念.md)

# 1. 模拟视频播放

-   [模拟视频播放](./doc/1.模拟视频播放.md)

详细代码见github:

-   [https://github.com/shizheng163/MyMediaPlayer/tree/v0.1.0](https://github.com/shizheng163/MyMediaPlayer/tree/v0.1.0)

# 2. opengl渲染yuv图像

-   [opengl渲染yuv图像](./doc/2.opengl渲染yuv图像.md)

详细代码见github:

-   [https://github.com/shizheng163/MyMediaPlayer/tree/v0.2.0](https://github.com/shizheng163/MyMediaPlayer/tree/v0.2.0)

文章中还提及了使用`ffmpeg`将视频拆分为多张yuv图像的方法。