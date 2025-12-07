#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QMap>
#include <QUrl>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QThread>
#include <QHash>
#include <QDateTime>

class AudioManager : public QObject
{
    Q_OBJECT

public:
    static AudioManager& getInstance();

    bool loadSounds();
    void playSound(const QString& soundName, float volume = 1.0f);
    void stopSound(const QString& soundName);
    void setMasterVolume(float volume);
    void setMusicVolume(float volume);
    void setEffectsVolume(float volume);

    void playBackgroundMusic();
    void stopBackgroundMusic();
    void playPlayerMove();
    void playArrowShot();
    void playEnemyHit();
    void playLevelUp();
    void playPlayerHurt();

    void stopAllSounds();
    void stopAllEffects();
    void cleanUp();
    void resetForNewLevel();

    void testAllSounds();

private:
    AudioManager();
    ~AudioManager() = default;

    void updateVolumes();

    struct SoundEffect {
        QMediaPlayer* player;
        QAudioOutput* output;
    };

    QMap<QString, SoundEffect> soundEffects;
    QMediaPlayer* backgroundMusic;
    QAudioOutput* musicOutput;

    float masterVolume;
    float musicVolume;
    float effectsVolume;
};

#endif // AUDIOMANAGER_H
