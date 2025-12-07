#include "uimanager.h"
#include <QDebug>
#include <QPainter>
#include <QFontDatabase>

UIManager& UIManager::getInstance()
{
    static UIManager instance;
    return instance;
}

bool UIManager::loadResources()
{
    qDebug() << "=== CARGANDO RECURSOS DE UI ===";

    fontLoaded = loadFont();

    cursorSprite = QPixmap(":/ui/ui/cursor.png");
    hudBackground = QPixmap(":/ui/ui/hud_background.png");
    buttonNormal = QPixmap(":/ui/ui/button_normal.png");
    buttonHover = QPixmap(":/ui/ui/button_hover.png");

    hudPanel = QPixmap(":/ui/ui/hud_background.png");
    ribbonRed = QPixmap(":/ui/ui/Ribbon_Red_3Slides.png");
    ribbonBlue = QPixmap(":/ui/ui/Ribbon_Blue_3Slides.png");
    ribbonGreen = QPixmap(":/ui/ui/Ribbon_Yellow_3Slides.png");
    hudSlide = QPixmap(":/ui/ui/hud_slide.png");
    hudRegular = QPixmap(":/ui/ui/hud_regular.png");

    if (cursorSprite.isNull()) qDebug() << "No se pudo cargar cursor.png";
    if (hudBackground.isNull()) qDebug() << "No se pudo cargar hud_background.png";
    if (buttonNormal.isNull()) qDebug() << "No se pudo cargar button_normal.png";
    if (buttonHover.isNull()) qDebug() << "No se pudo cargar button_hover.png";
    if (hudPanel.isNull()) qDebug() << "No se pudo cargar hud_panel.png";
    if (ribbonRed.isNull()) qDebug() << "No se pudo cargar ribbon_red.png";
    if (ribbonBlue.isNull()) qDebug() << "No se pudo cargar ribbon_blue.png";
    if (ribbonGreen.isNull()) qDebug() << "No se pudo cargar ribbon_green.png";
    if (hudSlide.isNull()) qDebug() << "No se pudo cargar hud_slide.png";
    if (hudRegular.isNull()) qDebug() << "No se pudo cargar hud_regular.png";

    if (!fontLoaded) {
        qDebug() << "No se pudo cargar la fuente TTF, usando fuentes del sistema";
    } else {
        qDebug() << "Fuente del juego cargada correctamente";
    }

    qDebug() << "=== RESUMEN DE CARGA DE RECURSOS UI ===";
    qDebug() << "Cursos:" << !cursorSprite.isNull();
    qDebug() << "Botones:" << (!buttonNormal.isNull() && !buttonHover.isNull());
    qDebug() << "Panel HUD:" << !hudPanel.isNull();
    qDebug() << "Ribbons:" << (!ribbonRed.isNull() && !ribbonBlue.isNull() && !ribbonGreen.isNull());
    qDebug() << "Slide HUD:" << !hudSlide.isNull();
    qDebug() << "Regular HUD:" << !hudRegular.isNull();
    qDebug() << "Fuente:" << fontLoaded;
    qDebug() << "========================================";

    return true;
}

bool UIManager::loadFont()
{
    int fontId = QFontDatabase::addApplicationFont(":/font/fonts/monogram.ttf");

    if (fontId == -1) {
        qDebug() << "No se pudo cargar monogram.ttf desde recursos";

        fontId = QFontDatabase::addApplicationFont("fonts/monogram.ttf");
        if (fontId == -1) {
            qDebug() << "No se pudo cargar monogram.ttf desde directorio fonts/";

            fontId = QFontDatabase::addApplicationFont(":/font/fonts/game_font.ttf");
            if (fontId == -1) {
                fontId = QFontDatabase::addApplicationFont("fonts/game_font.ttf");
                if (fontId == -1) {
                    qDebug() << "No se pudo cargar ninguna fuente TTF";
                    return false;
                }
            }
        }
    }

    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    if (fontFamilies.isEmpty()) {
        qDebug() << "No se pudieron obtener las familias de la fuente";
        return false;
    }

    QString fontFamily = fontFamilies.at(0);
    qDebug() << "Fuente cargada:" << fontFamily;

    gameFont.setFamily(fontFamily);
    gameFont.setPixelSize(16);
    gameFont.setBold(false);
    gameFont.setStyleStrategy(QFont::PreferAntialias);

    QFontMetrics metrics(gameFont);
    qDebug() << "Métricas de fuente - Altura:" << metrics.height()
             << "Ancho 'A':" << metrics.horizontalAdvance('A')
             << "Ascent:" << metrics.ascent() << "Descent:" << metrics.descent();

    return true;
}

void UIManager::drawText(QPainter& painter, const QString& text, int x, int y, float scale)
{
    if (text.isEmpty()) return;

    painter.save();

    if (fontLoaded) {
        QFont scaledFont = gameFont;
        scaledFont.setPixelSize(gameFont.pixelSize() * scale);
        painter.setFont(scaledFont);
        painter.setPen(QPen(Qt::white));
        painter.setPen(QPen(QColor(0, 0, 0, 180)));
        painter.drawText(x + 1, y + 1, text);
        painter.setPen(QPen(Qt::white));
        painter.drawText(x, y, text);
    } else {
        QFont systemFont = painter.font();
        systemFont.setPixelSize(12 * scale);
        systemFont.setBold(true);
        painter.setFont(systemFont);
        painter.setPen(QPen(QColor(0, 0, 0, 200), 3));
        painter.drawText(x, y, text);
        painter.setPen(QPen(Qt::white));
        painter.drawText(x, y, text);
    }

    painter.restore();
}

int UIManager::getTextWidth(const QString& text, float scale) const
{
    if (text.isEmpty()) return 0;

    if (fontLoaded) {
        QFontMetrics fontMetrics(gameFont);
        if (scale != 1.0f) {
            QFont scaledFont = gameFont;
            scaledFont.setPixelSize(gameFont.pixelSize() * scale);
            QFontMetrics scaledMetrics(scaledFont);
            return scaledMetrics.horizontalAdvance(text);
        }
        return fontMetrics.horizontalAdvance(text);
    } else {
        QFont systemFont;
        systemFont.setPixelSize(12 * scale);
        systemFont.setBold(true);
        QFontMetrics fontMetrics(systemFont);
        return fontMetrics.horizontalAdvance(text);
    }
}

int UIManager::getTextHeight(float scale) const
{
    if (fontLoaded) {
        QFontMetrics fontMetrics(gameFont);
        if (scale != 1.0f) {
            QFont scaledFont = gameFont;
            scaledFont.setPixelSize(gameFont.pixelSize() * scale);
            QFontMetrics scaledMetrics(scaledFont);
            return scaledMetrics.height();
        }
        return fontMetrics.height();
    } else {
        QFont systemFont;
        systemFont.setPixelSize(12 * scale);
        systemFont.setBold(true);
        QFontMetrics fontMetrics(systemFont);
        return fontMetrics.height();
    }
}
