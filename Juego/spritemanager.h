#ifndef SPRITEMANAGER_H
#define SPRITEMANAGER_H

#include <QObject>
#include <QPixmap>
#include <QMap>
#include <QString>
#include <QSize>

class SpriteManager : public QObject
{
    Q_OBJECT

public:
    static SpriteManager& getInstance();

    bool loadSprite(const QString& name, const QString& filePath);
    bool loadSpriteSheet(const QString& name, const QString& filePath, const QSize& frameSize);
    QPixmap getSprite(const QString& name) const;
    QPixmap getSpriteFrame(const QString& sheetName, int frameIndex) const;
    bool contains(const QString& name) const;

    void preloadGameSprites();

private:
    SpriteManager() = default;
    ~SpriteManager() = default;

    QMap<QString, QPixmap> sprites;
    QMap<QString, QPixmap> spriteSheets;
    QMap<QString, QSize> spriteSheetFrameSizes;
};

#endif // SPRITEMANAGER_H
