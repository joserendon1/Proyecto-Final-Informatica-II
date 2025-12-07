#include "mapa.h"
#include <QDebug>
#include <QRandomGenerator>

Mapa::Mapa(QObject *parent) : QObject(parent)
{
    posicionInicio = QPointF(100, 100);
    cargarSpritesMapa();
}

Mapa::~Mapa()
{
    capaColisiones.clear();
    elementosMapa.clear();
}

void Mapa::cargarSpritesMapa()
{
    spriteSuelo = QPixmap(":/map/maps/ground.png");
    spriteHouse = QPixmap(":/map/maps/House.png");
    spriteTower = QPixmap(":/map/maps/Tower.png");
    spriteRoca = QPixmap(":/map/maps/Rock.png");
    spriteTreeSheet = QPixmap(":/map/maps/Tree.png");
    spriteBushSheet = QPixmap(":/map/maps/Bushe.png");

    if(spriteSuelo.isNull()) {
        qDebug() << "No se pudo cargar ground.png";
        spriteSuelo = QPixmap(64, 64);
        spriteSuelo.fill(QColor(120, 120, 140));
    }

    qDebug() << "   Sprites del mapa cargados:";
    qDebug() << "   Suelo:" << spriteSuelo.size();
    qDebug() << "   Casa:" << spriteHouse.size();
    qDebug() << "   Torre:" << spriteTower.size();
    qDebug() << "   Roca:" << spriteRoca.size();
    qDebug() << "   Árbol (sheet):" << spriteTreeSheet.size() << "- 8 frames de 192x256";
    qDebug() << "   Arbusto (sheet):" << spriteBushSheet.size() << "- 8 frames de 128x128";
}

void Mapa::crearMapaGrande(const QSize& tamano)
{
    QPixmap mapaTemp(tamano);
    mapaTemp.fill(Qt::transparent);

    QPainter painter(&mapaTemp);
    painter.setRenderHint(QPainter::Antialiasing);

    int tileSize = 64;
    int tilesX = tamano.width() / tileSize;
    int tilesY = tamano.height() / tileSize;

    qDebug() << "Creando mapa decorativo con tiles de suelo:" << tilesX << "x" << tilesY;

    for(int x = 0; x <= tilesX; x++) {
        for(int y = 0; y <= tilesY; y++) {
            QRectF tileRect(x * tileSize, y * tileSize, tileSize, tileSize);
            painter.drawPixmap(tileRect, spriteSuelo, spriteSuelo.rect());
        }
    }

    elementosMapa.clear();

    int centroX = tamano.width() / 2;
    int centroY = tamano.height() / 2;

    for(int i = 0; i < 40; i++) {
        int x, y;
        do {
            x = QRandomGenerator::global()->bounded(200, tamano.width() - 200);
            y = QRandomGenerator::global()->bounded(200, tamano.height() - 200);
        } while (qAbs(x - centroX) < 300 && qAbs(y - centroY) < 300);

        ElementoMapa arbol;
        arbol.sprite = spriteTreeSheet;
        arbol.posicion = QPointF(x, y);
        arbol.areaColision = QRectF(0, 0, 0, 0);
        arbol.esDecoracion = true;
        arbol.esAnimado = true;
        arbol.frameActual = QRandomGenerator::global()->bounded(8);
        arbol.totalFrames = 8;
        arbol.frameWidth = 192;
        arbol.frameHeight = 256;
        arbol.tiempoPorFrame = 100.0f;
        arbol.tiempoAcumulado = 0;
        elementosMapa.append(arbol);
    }

    for(int i = 0; i < 60; i++) {
        int x = QRandomGenerator::global()->bounded(100, tamano.width() - 100);
        int y = QRandomGenerator::global()->bounded(100, tamano.height() - 100);

        ElementoMapa arbusto;
        arbusto.sprite = spriteBushSheet;
        arbusto.posicion = QPointF(x, y);
        arbusto.areaColision = QRectF(0, 0, 0, 0);
        arbusto.esDecoracion = true;
        arbusto.esAnimado = true;
        arbusto.frameActual = QRandomGenerator::global()->bounded(8);
        arbusto.totalFrames = 8;
        arbusto.frameWidth = 128;
        arbusto.frameHeight = 128;
        arbusto.tiempoPorFrame = 120.0f;
        arbusto.tiempoAcumulado = 0;
        elementosMapa.append(arbusto);
    }

    painter.end();

    mapaCompleto = mapaTemp;

    qDebug() << "️ Mapa decorativo creado con" << elementosMapa.size() << "elementos";
    qDebug() << "   - Árboles animados: 40";
    qDebug() << "   - Arbustos animados: 60";

    procesarColisionesDesdeMapa();
    posicionInicio = QPointF(centroX, centroY);
}

void Mapa::actualizarAnimaciones(float deltaTime)
{
    for(ElementoMapa& elemento : elementosMapa) {
        if(elemento.esAnimado) {
            elemento.tiempoAcumulado += deltaTime;

            if(elemento.tiempoAcumulado >= elemento.tiempoPorFrame) {
                elemento.frameActual = (elemento.frameActual + 1) % elemento.totalFrames;
                elemento.tiempoAcumulado = 0;
            }
        }
    }
}

QPixmap Mapa::obtenerFrameAnimado(const ElementoMapa& elemento) const
{
    if(!elemento.esAnimado || elemento.sprite.isNull()) {
        return elemento.sprite;
    }

    int frameX = elemento.frameActual * elemento.frameWidth;

    if(frameX + elemento.frameWidth > elemento.sprite.width()) {
        return elemento.sprite;
    }

    QRect frameRect(frameX, 0, elemento.frameWidth, elemento.frameHeight);
    return elemento.sprite.copy(frameRect);
}

void Mapa::dibujarElementosMapa(QPainter& painter, const QRectF& vista)
{
    for(const ElementoMapa& elemento : elementosMapa) {
        int drawWidth = elemento.esAnimado ? elemento.frameWidth : elemento.sprite.width();
        int drawHeight = elemento.esAnimado ? elemento.frameHeight : elemento.sprite.height();

        QRectF elementoRect(elemento.posicion.x() - drawWidth/2,
                            elemento.posicion.y() - drawHeight/2,
                            drawWidth,
                            drawHeight);

        if(vista.intersects(elementoRect)) {
            QPointF posRelativa = elemento.posicion - vista.topLeft();
            QRectF drawRect(posRelativa.x() - drawWidth/2,
                            posRelativa.y() - drawHeight/2,
                            drawWidth,
                            drawHeight);

            QPixmap spriteADibujar = elemento.esAnimado ?
                                         obtenerFrameAnimado(elemento) :
                                         elemento.sprite;

            painter.drawPixmap(drawRect, spriteADibujar, spriteADibujar.rect());
        }
    }
}

void Mapa::procesarColisionesDesdeMapa()
{
    if(mapaCompleto.isNull()) return;

    QImage imagen = mapaCompleto.toImage();
    int ancho = imagen.width();
    int alto = imagen.height();

    capaColisiones.clear();
    capaColisiones.resize(alto);

    for(int y = 0; y < alto; y++) {
        capaColisiones[y].resize(ancho);
        for(int x = 0; x < ancho; x++) {
            capaColisiones[y][x] = false;
        }
    }

    qDebug() << " Mapa completamente transitable - Sin obstáculos de colisión";
}

void Mapa::dibujar(QPainter& painter, const QRectF& vista)
{
    if(mapaCompleto.isNull()) {
        painter.fillRect(vista, QColor(60, 60, 80));
        return;
    }

    painter.drawPixmap(QRectF(0, 0, vista.width(), vista.height()),
                       mapaCompleto,
                       vista);

    dibujarElementosMapa(painter, vista);
}

bool Mapa::esTransitable(int x, int y) const
{
    if(x < 0 || y < 0 || x >= mapaCompleto.width() || y >= mapaCompleto.height()) {
        return false;
    }

    if(y < capaColisiones.size() && x < capaColisiones[y].size()) {
        return !capaColisiones[y][x];
    }

    return true;
}

QRectF Mapa::getLimitesMapa() const
{
    return QRectF(0, 0, mapaCompleto.width(), mapaCompleto.height());
}

QPointF Mapa::getPosicionInicioJugador() const {
    return posicionInicio;
}
