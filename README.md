# 简单的音乐播放器

南京邮电大学程序设计（上机）95分优秀作业，祝学弟学妹上机课人人95！

为了方便你无阻碍地跑通代码，请务必阅读如下注意事项

1、确保你的QT环境是QT6，而不是QT5，因为代码用到了multimedia库，这个是新版QT才有的

2、musicplayer.cpp文件中第75行代码为硬编码，需要根据实际情况修改，我写的是把文件直接放在D盘的情况

即如果碰到歌曲播放不出来的情况，修改这一行代码就可以了
// 指定默认的音乐文件夹路径
QString defaultPath ="D:/MusicPlayer/MusicPlayer/resource/resourcemusic";

3、参考的优化点如下，新增网络编程功能，新增数据库，调用ffmpeg的api取代multimedia库等。

如有任何问题，欢迎留言
