#include "audiomanager.h"
#include <QDebug>
#include <algorithm>
#include <QDir>
#include <QTimer>

AudioManager& AudioManager::getInstance()
{
    static AudioManager instance;
    return instance;
}

AudioManager::AudioManager() : QObject()
{
    backgroundMusic = nullptr;
    musicOutput = nullptr;
    masterVolume = 1.0f;
    musicVolume = 0.7f;
    effectsVolume = 0.8f;

    qDebug() << "AudioManager creado";
}

bool AudioManager::loadSounds()
{
    qDebug() << "=== CARGANDO SONIDOS ===";

    QList<QAudioDevice> devices = QMediaDevices::audioOutputs();
    if (devices.isEmpty()) {
        qDebug() << "No se encontraron dispositivos de audio - Continuando sin audio";
        return false;
    }

    qDebug() << "Dispositivos de audio disponibles:" << devices.size();

    if (!backgroundMusic) {
        backgroundMusic = new QMediaPlayer(this);
    }
    if (!musicOutput) {
        musicOutput = new QAudioOutput(this);
    }

    try {
        musicOutput->setDevice(QMediaDevices::defaultAudioOutput());
        backgroundMusic->setAudioOutput(musicOutput);

        QString musicPath = "qrc:/sounds/sounds/background_music.ogg";
        backgroundMusic->setSource(QUrl(musicPath));

        QThread::msleep(100);

        if (backgroundMusic->mediaStatus() == QMediaPlayer::NoMedia) {
            qDebug() << "No se pudo cargar la música de fondo";
        } else {
            qDebug() << "Música de fondo cargada correctamente";
            backgroundMusic->setLoops(QMediaPlayer::Infinite);
            musicOutput->setVolume(musicVolume * masterVolume);
        }
    } catch (...) {
        qDebug() << "Excepción al cargar música de fondo";
    }

    QMap<QString, QString> soundFiles = {
        {"arrow_shot", "qrc:/sounds/sounds/arrow_shot.ogg"},
        {"player_move", "qrc:/sounds/sounds/player_move.ogg"},
        {"enemy_hit", "qrc:/sounds/sounds/enemy_hit.ogg"},
        {"level_up", "qrc:/sounds/sounds/level_up.ogg"},
        {"button_click", "qrc:/sounds/sounds/button_click.ogg"},
        {"player_hurt", "qrc:/sounds/sounds/player_hurt.ogg"}
    };

    int loadedCount = 0;

    for (auto it = soundFiles.begin(); it != soundFiles.end(); ++it) {
        try {
            SoundEffect effect;
            effect.player = new QMediaPlayer(this);
            effect.output = new QAudioOutput(this);
            effect.output->setDevice(QMediaDevices::defaultAudioOutput());
            effect.player->setAudioOutput(effect.output);
            effect.player->setSource(QUrl(it.value()));
            QThread::msleep(50);
            if (effect.player->mediaStatus() != QMediaPlayer::NoMedia) {
                loadedCount++;
                qDebug() << "Sonido cargado:" << it.key();
                effect.output->setVolume(effectsVolume * masterVolume);
                soundEffects[it.key()] = effect;
            } else {
                qDebug() << "No se pudo cargar:" << it.key();
                delete effect.player;
                delete effect.output;
            }
        } catch (...) {
            qDebug() << "Excepción al cargar sonido:" << it.key();
        }
    }

    qDebug() << "=== CARGA COMPLETADA: " << loadedCount << "/" << soundFiles.size() << "sonidos ===";

    return loadedCount > 0 || backgroundMusic->mediaStatus() != QMediaPlayer::NoMedia;
}

void AudioManager::playSound(const QString& soundName, float volume)
{
    if (!soundEffects.contains(soundName)) {
        qDebug() << "Sonido no encontrado:" << soundName;
        return;
    }

    SoundEffect& effect = soundEffects[soundName];

    if (effect.player->source().isEmpty()) {
        qDebug() << " Sonido" << soundName << "no tiene fuente cargada";
        return;
    }

    static QHash<QString, qint64> ultimaReproduccion;
    qint64 ahora = QDateTime::currentMSecsSinceEpoch();

    if (ultimaReproduccion.contains(soundName)) {
        qint64 diferencia = ahora - ultimaReproduccion[soundName];
        if (diferencia < 100) {
            return;
        }
    }

    ultimaReproduccion[soundName] = ahora;
    float finalVolume = volume * effectsVolume * masterVolume;
    effect.output->setVolume(finalVolume);

    if (effect.player->playbackState() == QMediaPlayer::PlayingState) {
        effect.player->stop();
    }

    effect.player->setPosition(0);
    effect.player->play();

    qDebug() << "Reproduciendo:" << soundName << "volumen:" << finalVolume;
}

void AudioManager::stopSound(const QString& soundName)
{
    if (soundEffects.contains(soundName)) {
        soundEffects[soundName].player->stop();
        qDebug() << "Sonido detenido:" << soundName;
    }
}

void AudioManager::stopAllSounds()
{
    qDebug() << "Deteniendo todos los sonidos...";

    stopBackgroundMusic();

    stopAllEffects();

    qDebug() << "Todos los sonidos detenidos";
}

void AudioManager::stopAllEffects()
{
    qDebug() << "Deteniendo todos los efectos de sonido...";

    for (auto& sound : soundEffects) {
        if (sound.player && sound.player->playbackState() == QMediaPlayer::PlayingState) {
            sound.player->stop();
            sound.player->setPosition(0);
        }
    }

    qDebug() << "Todos los efectos de sonido detenidos";
}

void AudioManager::cleanUp()
{
    qDebug() << "Limpiando AudioManager...";

    stopAllSounds();

    for (auto& sound : soundEffects) {
        if (sound.player) {
            sound.player->stop();
            delete sound.player;
        }
        if (sound.output) {
            delete sound.output;
        }
    }
    soundEffects.clear();

    if (backgroundMusic) {
        backgroundMusic->stop();
        delete backgroundMusic;
        backgroundMusic = nullptr;
    }
    if (musicOutput) {
        delete musicOutput;
        musicOutput = nullptr;
    }

    qDebug() << "AudioManager limpiado completamente";
}

void AudioManager::resetForNewLevel()
{
    qDebug() << "Reiniciando AudioManager para nuevo nivel...";

    stopAllSounds();

    QTimer::singleShot(100, [this]() {
        loadSounds();
        qDebug() << "AudioManager reiniciado para nuevo nivel";
    });
}

void AudioManager::setMasterVolume(float volume)
{
    masterVolume = std::max(0.0f, std::min(volume, 1.0f));
    updateVolumes();
}

void AudioManager::setMusicVolume(float volume)
{
    musicVolume = std::max(0.0f, std::min(volume, 1.0f));
    updateVolumes();
}

void AudioManager::setEffectsVolume(float volume)
{
    effectsVolume = std::max(0.0f, std::min(volume, 1.0f));
    updateVolumes();
}

void AudioManager::updateVolumes()
{
    if (musicOutput) {
        musicOutput->setVolume(musicVolume * masterVolume);
        qDebug() << "Volumen música actualizado:" << musicVolume * masterVolume;
    }

    for (auto& effect : soundEffects) {
        if (effect.output) {
            effect.output->setVolume(effectsVolume * masterVolume);
        }
    }

    qDebug() << "Volumen efectos actualizado:" << effectsVolume * masterVolume;
}

void AudioManager::playBackgroundMusic()
{
    if (backgroundMusic && !backgroundMusic->source().isEmpty()) {
        qDebug() << "Iniciando música de fondo";
        backgroundMusic->play();
    } else {
        qDebug() << "No se puede reproducir música de fondo - no cargada";
    }
}

void AudioManager::stopBackgroundMusic()
{
    if (backgroundMusic) {
        backgroundMusic->stop();
        qDebug() << "Música de fondo detenida";
    }
}

void AudioManager::playPlayerMove()
{
    playSound("player_move", 0.6f);
}

void AudioManager::playArrowShot()
{
    playSound("arrow_shot", 0.8f);
}

void AudioManager::playEnemyHit()
{
    playSound("enemy_hit", 0.7f);
}

void AudioManager::playLevelUp()
{
    playSound("level_up", 1.0f);
}

void AudioManager::playPlayerHurt()
{
    playSound("player_hurt", 0.8f);
}

void AudioManager::testAllSounds()
{
    qDebug() << "=== TESTEANDO TODOS LOS SONIDOS ===";

    for (auto it = soundEffects.begin(); it != soundEffects.end(); ++it) {
        qDebug() << "Probando:" << it.key();
        playSound(it.key(), 0.5f);
        QThread::msleep(500);
    }

    qDebug() << "=== TEST COMPLETADO ===";
}
