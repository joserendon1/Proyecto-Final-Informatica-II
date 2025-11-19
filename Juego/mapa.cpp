#include "mapa.h"
#include <QDebug>
#include <QRandomGenerator>

Mapa::Mapa(QObject *parent) : QObject(parent)
{
    posicionInicio = QPointF(100, 100);
}

Mapa::~Mapa()
{
    capaColisiones.clear();
}

void Mapa::crearMapaGrande(const QSize& tamano)
{
    mapaCompleto = QPixmap(tamano);

    mapaCompleto.fill(QColor(120, 120, 140));

    QPainter painter(&mapaCompleto);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(QPen(QColor(100, 100, 120), 2));
    int gridSize = 200;
    for(int x = 0; x < tamano.width(); x += gridSize) {
        painter.drawLine(x, 0, x, tamano.height());
    }
    for(int y = 0; y < tamano.height(); y += gridSize) {
        painter.drawLine(0, y, tamano.width(), y);
    }

    painter.setBrush(QBrush(QColor(30, 30, 50)));
    painter.setPen(Qt::NoPen);

    int grosorBorde = 100;
    int ancho = tamano.width();
    int alto = tamano.height();

    painter.drawRect(0, 0, ancho, grosorBorde);
    painter.drawRect(0, alto - grosorBorde, ancho, grosorBorde);
    painter.drawRect(0, 0, grosorBorde, alto);
    painter.drawRect(ancho - grosorBorde, 0, grosorBorde, alto);

    painter.setBrush(QBrush(QColor(40, 40, 60)));
    painter.setPen(QPen(QColor(20, 20, 40), 3));

    painter.drawRect(ancho/2 - 150, alto/2 - 150, 300, 300);

    painter.drawRect(ancho * 0.1, alto * 0.1, 200, 200);
    painter.drawRect(ancho * 0.7, alto * 0.1, 200, 200);
    painter.drawRect(ancho * 0.1, alto * 0.7, 200, 200);
    painter.drawRect(ancho * 0.7, alto * 0.7, 200, 200);

    for(int i = 0; i < 10; i++) {
        int x = QRandomGenerator::global()->bounded(ancho - 100);
        int y = QRandomGenerator::global()->bounded(alto - 100);
        int w = 50 + QRandomGenerator::global()->bounded(100);
        int h = 50 + QRandomGenerator::global()->bounded(100);
        painter.drawRect(x, y, w, h);
    }

    painter.end();

    qDebug() << "🗺️ Mapa grande creado - Tamaño:" << mapaCompleto.size();

    procesarColisionesDesdeMapa();

    posicionInicio = QPointF(ancho / 2, alto / 2);
    qDebug() << "📍 Posición inicial del jugador:" << posicionInicio;
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
            QColor color = imagen.pixelColor(x, y);
            // Negro sólido = no transitable
            capaColisiones[y][x] = (color.red() == 0 && color.green() == 0 &&
                                    color.blue() == 0 && color.alpha() == 255);
        }
    }

    qDebug() << "🎯 Capa de colisiones procesada:" << ancho << "x" << alto;
}

void Mapa::dibujar(QPainter& painter, const QRectF& vista)
{
    if(mapaCompleto.isNull()) {
        painter.fillRect(vista, QColor(60, 60, 80));
        return;
    }
    painter.drawPixmap(vista, mapaCompleto, vista);
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
