#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include <QWidget>
#include <QUrl>
#include <QStandardItemModel>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QList>
#include <QVector>
#include <QSharedPointer>

QT_BEGIN_NAMESPACE
namespace Ui {
class MusicPlayer;
}
QT_END_NAMESPACE

class MusicPlayer : public QWidget
{
    Q_OBJECT

public:
    MusicPlayer(QWidget *parent = nullptr);
    ~MusicPlayer();


private slots:
    void on_pause_clicked();

    void on_preSong_clicked();

    void on_nextSong_clicked();

    void on_songList_doubleClicked(const QModelIndex &index);

    void on_playpaceSlider_sliderMoved();

    void on_volume_clicked();

    void on_verticalSlider_valueChanged(int value);

    void on_modelstage_clicked();

    void on_love_clicked();

    void on_favoritesList_doubleClicked(const QModelIndex &index);

    void loadMusicFiles(const QString &path);

    void playNextSong(int nextIndex);

    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);

    void updateMusicPicture(int nowindex);

private:
    Ui::MusicPlayer *ui;
    QList<QUrl> playlist;//双向列表
    QStringList loveNames;//收藏列表
    QScopedPointer<QMediaPlayer> mediaPlayer;//使用智能指针管理对象的生命周期
    QScopedPointer<QAudioOutput> audioOutput;
    int curindex =0;//初始标记为0
    QSharedPointer<int> stage = QSharedPointer<int>::create(0);//使用共享的智能指针
    QList<int> history;  // 存储历史记录（歌曲的索引）
    int maxHistorySize = 10; // 最大历史记录数量
    QVector<int> vecbool;//判断有没有加入收藏
    QVector<int> veclove;//提供收藏列表对应的歌曲列表索引节点


};
#endif // MUSICPLAYER_H
