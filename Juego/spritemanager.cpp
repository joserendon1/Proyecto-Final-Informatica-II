#include "spritemanager.h"
#include <QDebug>

SpriteManager& SpriteManager::getInstance()
{
    static SpriteManager instance;
    return instance;
}

bool SpriteManager::loadSprite(const QString& name, const QString& filePath)
{
    QPixmap sprite(filePath);
    if(sprite.isNull()) {
        qDebug() << "Error: No se pudo cargar el sprite:" << filePath;
        return false;
    }

    sprites[name] = sprite;
    return true;
}

bool SpriteManager::loadSpriteSheet(const QString& name, const QString& filePath, const QSize& frameSize)
{
    QPixmap spriteSheet(filePath);
    if(spriteSheet.isNull()) {
        qDebug() << "Error: No se pudo cargar la sprite sheet:" << filePath;
        return false;
    }

    spriteSheets[name] = spriteSheet;
    spriteSheetFrameSizes[name] = frameSize;
    return true;
}

QPixmap SpriteManager::getSprite(const QString& name) const
{
    return sprites.value(name, QPixmap());
}

QPixmap SpriteManager::getSpriteFrame(const QString& sheetName, int frameIndex) const
{
    if(!spriteSheets.contains(sheetName)) {
        return QPixmap();
    }

    QPixmap sheet = spriteSheets[sheetName];
    QSize frameSize = spriteSheetFrameSizes[sheetName];

    // Calcular cuántos frames hay por fila
    int framesPerRow = sheet.width() / frameSize.width();

    // Calcular posición del frame
    int row = frameIndex / framesPerRow;
    int col = frameIndex % framesPerRow;

    QRect frameRect(col * frameSize.width(),
                    row * frameSize.height(),
                    frameSize.width(),
                    frameSize.height());

    return sheet.copy(frameRect);
}

bool SpriteManager::contains(const QString& name) const
{
    return sprites.contains(name) || spriteSheets.contains(name);
}

void SpriteManager::preloadGameSprites()
{
    qDebug() << "=== CARGANDO SPRITES ===";

    // Sprites del jugador
    bool loaded1 = loadSprite("player_idle", ":/sprites/player/Warrior_Idle.png");
    bool loaded2 = loadSprite("player_move", ":/sprites/player/Warrior_Run.png");

    // Sprites de enemigos
    bool loaded3 = loadSprite("enemy_weak", ":/sprites/enemies/Lancer_Run.png");
    bool loaded4 = loadSprite("enemy_strong", ":/sprites/enemies/Warrior_Run.png");

    // === SPRITES PARA ATAQUES ===
    // Espada - sprite sheet
    bool loaded5 = loadSpriteSheet("sword_slash", ":/sprites/attacks/sword.png", QSize(32, 32));

    // MISMO SPRITE para ballesta y arco
    bool loaded6 = loadSprite("projectile_arrow", ":/sprites/attacks/arrow.png");

    // Aceite
    bool loaded7 = loadSpriteSheet("oil_effect", ":/sprites/attacks/oil.png", QSize(96, 96));

    qDebug() << "=== ESTADO DE CARGA DE SPRITES ===";
    qDebug() << "Jugador (idle/move):" << loaded1 << loaded2;
    qDebug() << "Enemigos (weak/strong):" << loaded3 << loaded4;
    qDebug() << "Ataques (espada/flecha/aceite):" << loaded5 << loaded6 << loaded7;

    // VERIFICAR SPRITES CARGADOS
    QPixmap oilSheet = getSprite("oil_effect");
    qDebug() << "Sprite sheet 'oil_effect' cargado:" << !oilSheet.isNull();
    if(!oilSheet.isNull()) {
        qDebug() << "Tamaño del sheet:" << oilSheet.size();
        qDebug() << "Frames calculados: 9 frames de 96x96";

        // Verificar un frame específico
        QPixmap testFrame = getSpriteFrame("oil_effect", 0);
        qDebug() << "Frame 0 cargado:" << !testFrame.isNull();
        if(!testFrame.isNull()) {
            qDebug() << "Tamaño del frame:" << testFrame.size();
        }
    }
}
