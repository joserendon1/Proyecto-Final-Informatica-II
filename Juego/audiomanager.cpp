#include "audiomanager.h"
#include <QDebug>
#include <algorithm>
#include <QDir>

AudioManager& AudioManager::getInstance()
{
    static AudioManager instance;
    return instance;
}

AudioManager::AudioManager() : QObject()
{
    // Inicializar punteros a nullptr
    backgroundMusic = nullptr;
    musicOutput = nullptr;

    // Inicializar volúmenes
    masterVolume = 1.0f;
    musicVolume = 0.7f;
    effectsVolume = 0.8f;

    qDebug() << "🔊 AudioManager creado";
}

bool AudioManager::loadSounds()
{
    qDebug() << "=== CARGANDO SONIDOS ===";

    // VERIFICAR DISPOSITIVOS DE AUDIO PRIMERO
    QList<QAudioDevice> devices = QMediaDevices::audioOutputs();
    if (devices.isEmpty()) {
        qDebug() << "❌ No se encontraron dispositivos de audio - Continuando sin audio";
        return false;
    }

    qDebug() << "Dispositivos de audio disponibles:" << devices.size();

    // INICIALIZAR PUNTEROS CON SEGURIDAD
    if (!backgroundMusic) {
        backgroundMusic = new QMediaPlayer(this);
    }
    if (!musicOutput) {
        musicOutput = new QAudioOutput(this);
    }

    // CONFIGURAR MÚSICA CON VERIFICACIÓN DE ERRORES
    try {
        musicOutput->setDevice(QMediaDevices::defaultAudioOutput());
        backgroundMusic->setAudioOutput(musicOutput);

        QString musicPath = "qrc:/sounds/sounds/background_music.ogg";
        backgroundMusic->setSource(QUrl(musicPath));

        // Esperar un momento para que cargue
        QThread::msleep(100);

        if (backgroundMusic->mediaStatus() == QMediaPlayer::NoMedia) {
            qDebug() << "❌ No se pudo cargar la música de fondo";
            // NO eliminar los objetos, solo marcar como fallido
        } else {
            qDebug() << "✅ Música de fondo cargada correctamente";
            backgroundMusic->setLoops(QMediaPlayer::Infinite);
            musicOutput->setVolume(musicVolume * masterVolume);
        }
    } catch (...) {
        qDebug() << "❌ Excepción al cargar música de fondo";
    }

    // CARGAR EFECTOS DE SONIDO CON MÁS SEGURIDAD
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

            // Configurar con manejo de errores
            effect.output->setDevice(QMediaDevices::defaultAudioOutput());
            effect.player->setAudioOutput(effect.output);
            effect.player->setSource(QUrl(it.value()));

            // Pequeña pausa para la carga
            QThread::msleep(50);

            if (effect.player->mediaStatus() != QMediaPlayer::NoMedia) {
                loadedCount++;
                qDebug() << "✅ Sonido cargado:" << it.key();
                effect.output->setVolume(effectsVolume * masterVolume);
                soundEffects[it.key()] = effect;
            } else {
                qDebug() << "❌ No se pudo cargar:" << it.key();
                delete effect.player;
                delete effect.output;
            }
        } catch (...) {
            qDebug() << "❌ Excepción al cargar sonido:" << it.key();
        }
    }

    qDebug() << "=== CARGA COMPLETADA: " << loadedCount << "/" << soundFiles.size() << "sonidos ===";

    // Considerar éxito incluso si solo algunos sonidos cargaron
    return loadedCount > 0 || backgroundMusic->mediaStatus() != QMediaPlayer::NoMedia;
}

void AudioManager::playSound(const QString& soundName, float volume)
{
    if (!soundEffects.contains(soundName)) {
        qDebug() << "⚠️ Sonido no encontrado:" << soundName;
        return;
    }

    SoundEffect& effect = soundEffects[soundName];

    // Verificar que el sonido esté cargado
    if (effect.player->source().isEmpty()) {
        qDebug() << "⚠️ Sonido" << soundName << "no tiene fuente cargada";
        return;
    }

    // LIMITAR reproducciones muy seguidas del mismo sonido
    static QHash<QString, qint64> ultimaReproduccion;
    qint64 ahora = QDateTime::currentMSecsSinceEpoch();

    if (ultimaReproduccion.contains(soundName)) {
        qint64 diferencia = ahora - ultimaReproduccion[soundName];
        if (diferencia < 100) { // Mínimo 100ms entre reproducciones del mismo sonido
            return;
        }
    }

    ultimaReproduccion[soundName] = ahora;

    // Configurar volumen
    float finalVolume = volume * effectsVolume * masterVolume;
    effect.output->setVolume(finalVolume);

    // Si ya se está reproduciendo, reiniciar
    if (effect.player->playbackState() == QMediaPlayer::PlayingState) {
        effect.player->stop();
    }

    // Reiniciar y reproducir
    effect.player->setPosition(0);
    effect.player->play();

    qDebug() << "🔊 Reproduciendo:" << soundName << "volumen:" << finalVolume;
}

void AudioManager::stopSound(const QString& soundName)
{
    if (soundEffects.contains(soundName)) {
        soundEffects[soundName].player->stop();
    }
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
    // Actualizar volumen de música
    if (musicOutput) {
        musicOutput->setVolume(musicVolume * masterVolume);
        qDebug() << "🎵 Volumen música actualizado:" << musicVolume * masterVolume;
    }

    // Actualizar volumen de efectos
    for (auto& effect : soundEffects) {
        effect.output->setVolume(effectsVolume * masterVolume);
    }

    qDebug() << "🔊 Volumen efectos actualizado:" << effectsVolume * masterVolume;
}

// MÉTODOS ESPECÍFICOS
void AudioManager::playBackgroundMusic()
{
    if (backgroundMusic && !backgroundMusic->source().isEmpty()) {
        qDebug() << "🎵 Iniciando música de fondo";
        backgroundMusic->play();
    } else {
        qDebug() << "❌ No se puede reproducir música de fondo - no cargada";
    }
}

void AudioManager::stopBackgroundMusic()
{
    if (backgroundMusic) {
        backgroundMusic->stop();
        qDebug() << "🎵 Música de fondo detenida";
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

// MÉTODO DE DIAGNÓSTICO
void AudioManager::testAllSounds()
{
    qDebug() << "=== TESTEANDO TODOS LOS SONIDOS ===";

    for (auto it = soundEffects.begin(); it != soundEffects.end(); ++it) {
        qDebug() << "Probando:" << it.key();
        playSound(it.key(), 0.5f);
        QThread::msleep(500); // Pequeña pausa entre sonidos
    }

    qDebug() << "=== TEST COMPLETADO ===";
}
