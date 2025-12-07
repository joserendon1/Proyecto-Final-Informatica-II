#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <QObject>
#include <QPixmap>
#include <QMap>
#include <QChar>
#include <QFont>
#include <QFontDatabase>

class UIManager : public QObject
{
    Q_OBJECT

public:
    static UIManager& getInstance();
    bool loadResources();
    QPixmap getCursor() const { return cursorSprite; }
    QPixmap getHudBackground() const { return hudBackground; }
    QPixmap getButtonNormal() const { return buttonNormal; }
    QPixmap getButtonHover() const { return buttonHover; }
    QPixmap getHudPanel() const { return hudPanel; }
    QPixmap getRibbonRed() const { return ribbonRed; }
    QPixmap getRibbonBlue() const { return ribbonBlue; }
    QPixmap getRibbonGreen() const { return ribbonGreen; }
    QPixmap getHudSlide() const { return hudSlide; }
    QPixmap getHudRegular() const { return hudRegular; }
    void drawText(QPainter& painter, const QString& text, int x, int y, float scale = 1.0f);
    int getTextWidth(const QString& text, float scale = 1.0f) const;
    int getTextHeight(float scale = 1.0f) const;

private:
    UIManager() = default;
    ~UIManager() = default;
    bool loadFont();
    QPixmap cursorSprite;
    QPixmap hudBackground;
    QPixmap buttonNormal;
    QPixmap buttonHover;
    QPixmap hudPanel;
    QPixmap ribbonRed;
    QPixmap ribbonBlue;
    QPixmap ribbonGreen;
    QPixmap hudSlide;
    QPixmap hudRegular;
    QFont gameFont;
    bool fontLoaded;
    QPixmap fontSpriteSheet;
    int fontCharWidth;
    int fontCharHeight;
};

#endif // UIMANAGER_H
