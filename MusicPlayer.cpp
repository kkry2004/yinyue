#include "MusicPlayer.h"
#include "ui_MusicPlayer.h"
#include <QDebug>
#include <QFileDialog>
#include <QDir>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QString>
#include <QUrl>
#include <QDirIterator>
#include <QRandomGenerator>
#include <QList>
#include <QSharedPointer>

MusicPlayer::MusicPlayer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MusicPlayer)
{
    ui->setupUi(this);
    ui->verticalSlider->setHidden(true);
    //使用 QScopedPointer自动管理mediaPlayer和audioOutput
    mediaPlayer.reset(new QMediaPlayer(this));  //使用reset并将新的QMediaPlayer实例绑定到mediaPlayer指针上
    audioOutput.reset(new QAudioOutput(this));
    mediaPlayer->setAudioOutput(audioOutput.data());  //将 audioOutput传给mediaPlayer
    //connect(sender, signal发出的信号, receiver, slot槽函数)
    //展示播放文件总时长
    //使用lambda表达式获取duration
    connect(mediaPlayer.get(),&QMediaPlayer::durationChanged,this,[=](qint64 duration){
        ui->totallabel->setText(QString("%1:%2").arg(duration/1000/60,2,10,QChar('0')).arg(duration/1000%60,2,10,QChar('0')));
        //给播放进度条滑块设置总时长
        ui->playpaceSlider->setRange(0,duration);
    });
    //展示播放文件当前时长
    connect(mediaPlayer.get(),&QMediaPlayer::positionChanged,this,[=](qint64 position){
        ui->curlabel->setText(QString("%1:%2").arg(position/1000/60,2,10,QChar('0')).arg(position/1000%60,2,10,QChar('0')));
        //给播放进度条滑块设置当前进度
        ui->playpaceSlider->setValue(position);
        //自动下一曲
        if (position == mediaPlayer->duration()) {
            if (ui->songList->currentRow() == -1) {
                return;
            }
            switch (*stage) {
            case 0: {//顺序循环
                curindex = (++curindex) % playlist.size(); //下一首
                ui->songList->setCurrentRow(curindex);//选中对应歌曲，但不播放，仅ui变化
                playNextSong(curindex);//加入历史记录
                mediaPlayer->setSource(playlist[curindex]);
                mediaPlayer->play();
                break;
            }
            case 1: {//单曲循环
                //mediaPlayer->play(); //播放当前歌曲，下面的-1已经无限循环了
                break;
            }
            case 2: {//随机播放
                //生成一个介于0和playlist.size()-1之间的随机整数
                curindex = QRandomGenerator::global()->bounded(playlist.size());
                ui->songList->setCurrentRow(curindex);
                playNextSong(curindex);
                mediaPlayer->setSource(playlist[curindex]);
                mediaPlayer->play();
                break;
            }
            }
        }
    });

    connect(mediaPlayer.get(), &QMediaPlayer::mediaStatusChanged, this, &MusicPlayer::onMediaStatusChanged);

    // 设置初始音量
    audioOutput->setVolume(0.5);

    // 指定默认的音乐文件夹路径
    QString defaultPath = "D:/MusicPlayer/MusicPlayer/resource/resourcemusic";

    // 加载音乐文件
    loadMusicFiles(defaultPath);

    //加载收藏夹列表，设置为空
    for (int i = 0; i < playlist.size(); ++i) {
        vecbool.append(0);  // 动态添加元素
    }

    history.clear();
}

//加入历史记录，保证随机播放时回溯
void MusicPlayer::playNextSong(int curindex)
{
    // 如果历史记录超过最大长度，删除最旧的记录
    if (history.size() >= maxHistorySize) {
        history.removeFirst();  // 删除最旧的歌曲
    }
    history.append(curindex);  // 将当前歌曲索引添加到历史记录
}

MusicPlayer::~MusicPlayer()
{
    delete ui;
}

//加载播放列表
void MusicPlayer::loadMusicFiles(const QString &path)
{
    //检查路径是否有效
    if (path.isEmpty()) {
        return;
    }

    //遍历指定文件夹中的音乐文件
    QDirIterator it(path, {"*.mp3", "*.wav", "*.ogg"}, QDir::Files); // 只遍历文件
    while (it.hasNext()) {
        it.next(); //移动到下一个文件
        QFileInfo info = it.fileInfo(); //获取文件信息
        //将完整路径转为 QUrl 并添加到播放列表
        //获取文件的标准化路径，为了播放歌曲
        playlist.append(QUrl::fromLocalFile(info.canonicalFilePath()));
    }

    //更新UI中的播放列表
    QStringList songNames;
    for (const QUrl &url : playlist) {
        //为了显示歌曲名
        songNames.append(url.fileName()); //获取文件名
    }
    ui->songList->addItems(songNames); //更新歌曲列表显示
}

//播放和暂停
void MusicPlayer::on_pause_clicked()
{
    QModelIndex currentIndex = ui->songList->selectionModel()->currentIndex();
    int currentRow = currentIndex.row();  //获取当前选中的行号

    //无播放文件时不响应
    if (currentRow == -1) {
        return;
    }

    //获取选中的音乐文件路径
    curindex = currentRow; //更新当前索引
    QUrl selectedSong = playlist[curindex]; //获取选中的歌曲 URL

    //不同状态对应不同的操作
    switch (mediaPlayer->playbackState()) {
    case QMediaPlayer::PlaybackState::StoppedState: {
        //设置音频源并播放
        mediaPlayer->setSource(selectedSong);
        mediaPlayer->play();
        ui->pause->setStyleSheet("icon: url(:/resource/playing.png);");
        break;
    }
    case QMediaPlayer::PlaybackState::PausedState: {
        //继续播放
        mediaPlayer->play();
        ui->pause->setStyleSheet("icon: url(:/resource/playing.png);");
        break;
    }
    case QMediaPlayer::PlaybackState::PlayingState: {
        //暂停播放
        mediaPlayer->pause();
        ui->pause->setStyleSheet("icon: url(:/resource/pause.png);");
        break;
    }
    }
}

//上一曲
void MusicPlayer::on_preSong_clicked()
{
    if(ui->songList->currentRow()==-1){
        return;
    }
    switch (*stage) {
    case 0:{
        curindex=(--curindex+playlist.size())%playlist.size();
        playNextSong(curindex);
        break;
    }
    case 1:{
        break;
    }
    case 2:{
        history.removeLast();//删除当前歌曲的历史记录
        if (history.isEmpty()) {
            history.append(0); //如果没有历史记录，默认第一首歌
        }
        curindex = history.takeLast();
        playNextSong(curindex);
        break;
    }
    }
    ui->songList->setCurrentRow(curindex);
    mediaPlayer->setSource(playlist[curindex]);
    mediaPlayer->play();
    updateMusicPicture(curindex);
}

//下一曲
void MusicPlayer::on_nextSong_clicked()
{
    if(ui->songList->currentRow()==-1){
        return;
    }
    switch (*stage) {
    case 0:{
        curindex=(++curindex)%playlist.size();
        playNextSong(curindex);
        break;
    }
    case 1:{
        break;
    }
    case 2:{
        curindex = QRandomGenerator::global()->bounded(playlist.size());
        playNextSong(curindex);
        break;
    }
    }
    ui->songList->setCurrentRow(curindex);
    mediaPlayer->setSource(playlist[curindex]);
    mediaPlayer->play();
    updateMusicPicture(curindex);
}


//双击列表播放
void MusicPlayer::on_songList_doubleClicked(const QModelIndex &index)
{
    curindex=index.row();
    playNextSong(curindex);
    mediaPlayer->setSource(playlist[curindex]);
    updateMusicPicture(curindex);
    mediaPlayer->play();
    ui->pause->setStyleSheet("icon: url(:/resource/playing.png);");
}

//随播放进度条滑块移动改变播放进度
void MusicPlayer::on_playpaceSlider_sliderMoved()
{
    mediaPlayer->setPosition(ui->playpaceSlider->sliderPosition());
}

//点击音量按钮调出/隐藏音量调节滑块
void MusicPlayer::on_volume_clicked()
{
    if(ui->verticalSlider->isHidden()){
        ui->verticalSlider->setHidden(false);
    }
    else{
        ui->verticalSlider->setHidden(true);
    }
}
//随音量滑块移动调节音量
void MusicPlayer::on_verticalSlider_valueChanged(int value)
{
    audioOutput->setVolume(float(value)/100);
}


//模式切换
void MusicPlayer::on_modelstage_clicked()
{
    switch (*stage) {
    case 0: { //顺序循环
        *stage = 1; //切换到单曲循环
        ui->modelstage->setStyleSheet("icon: url(:/resource/songrepeat.png);"); // 单曲循环图标
        mediaPlayer->setLoops(-1); //设置为单曲循环
        break;
    }
    case 1: { // 单曲循环
        *stage = 2; //切换到随机播放
        ui->modelstage->setStyleSheet("icon: url(:/resource/listrandom.png);"); // 随机播放图标
        mediaPlayer->setLoops(1); //设置为无限循环
        break;
    }
    case 2: { //随机播放
        *stage = 0; //切换到顺序循环
        ui->modelstage->setStyleSheet("icon: url(:/resource/listrepeat.png);"); // 顺序循环图标
        mediaPlayer->setLoops(1); //设置为顺序播放一次
        break;
    }
    }
}

//收藏和取消功能
void MusicPlayer::on_love_clicked()
{
    //判断当前索引是否合法
    if (curindex >= playlist.size()) {
        return; //索引越界，直接返回
    }

    // 加入收藏
    if (vecbool[curindex] == 0) {
        vecbool[curindex] = 1;  // 将当前索引位置的元素设置为喜欢

        //如果当前歌曲没有在 loveNames 中，则加入
        if (!loveNames.contains(playlist[curindex].fileName())) {
            loveNames.append(playlist[curindex].fileName());  //插入 playlist 对应索引的文件名
            veclove.append(curindex);  //将当前索引添加到收藏列表
        }
    } else {  //取消收藏
        vecbool[curindex] = 0;  //将当前索引位置的元素取消喜欢
        //倒序遍历 loveNames 列表，防止删除时引起索引问题
        for (int i = loveNames.size() - 1; i >= 0; --i) {
            if (loveNames[i] == playlist[curindex].fileName()) {
                loveNames.removeAt(i);  //从列表中移除该歌曲
                veclove.removeAt(i);    //从收藏列表中删除
                break; //只删除第一次出现的匹配项
            }
        }
    }

    //更新 favoritesList 显示
    ui->favoritesList->clear();  //清空原来的列表
    ui->favoritesList->addItems(loveNames);  //一次性添加更新后的列表
}

//双击收藏列表播放
void MusicPlayer::on_favoritesList_doubleClicked(const QModelIndex &index)
{
    curindex = veclove[index.row()];  //获取收藏列表中的歌曲索引
    playNextSong(curindex);
    mediaPlayer->setSource(playlist[curindex]);
    mediaPlayer->play();
    updateMusicPicture(curindex);
    ui->pause->setStyleSheet("icon: url(:/resource/playing.png);");
}

//音乐自然播放完毕时触发信号
void MusicPlayer::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
    if (status == QMediaPlayer::EndOfMedia) {
        updateMusicPicture(curindex);
    }
}

void MusicPlayer::updateMusicPicture(int nowindex)
{
    //根据 curindex 的值动态构建图片路径
    QString imagePath = QString(":/resource/musicpicture/%1.png").arg(nowindex);
    //qDebug() << "Image path: " << imagePath;
    //设置图片路径到 musicPicture 控件
    ui->musicPicture->setStyleSheet(QString("image: url(%1);").arg(imagePath));
}

