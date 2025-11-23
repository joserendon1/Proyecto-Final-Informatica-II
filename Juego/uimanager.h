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

    // Getters para recursos
    QPixmap getCursor() const { return cursorSprite; }
    QPixmap getHudBackground() const { return hudBackground; }
    QPixmap getButtonNormal() const { return buttonNormal; }
    QPixmap getButtonHover() const { return buttonHover; }

    // Nuevos getters para sprites de UI
    QPixmap getHudPanel() const { return hudPanel; }
    QPixmap getRibbonRed() const { return ribbonRed; }
    QPixmap getRibbonBlue() const { return ribbonBlue; }
    QPixmap getRibbonGreen() const { return ribbonGreen; }
    QPixmap getHudSlide() const { return hudSlide; }
    QPixmap getHudRegular() const { return hudRegular; }

    // Métodos para texto
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

    // Nuevos sprites de UI
    QPixmap hudPanel;      // 192x192 - fondo del HUD
    QPixmap ribbonRed;     // 192x64 - ribbon rojo
    QPixmap ribbonBlue;    // 192x64 - ribbon azul
    QPixmap ribbonGreen;   // 192x64 - ribbon verde
    QPixmap hudSlide;      // 192x64 - slide del HUD
    QPixmap hudRegular;    // 64x64  - panel regular

    // Para fuente TTF
    QFont gameFont;
    bool fontLoaded;

    // Variables legacy para bitmap font
    QPixmap fontSpriteSheet;
    int fontCharWidth;
    int fontCharHeight;
};

#endif // UIMANAGER_H
