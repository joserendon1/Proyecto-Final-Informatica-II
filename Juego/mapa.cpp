#include "mapa.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QRandomGenerator>
#include <QImage>

Mapa::Mapa(QObject *parent) : QObject(parent)
{
    posicionInicio = QPointF(100, 100);
}

Mapa::~Mapa()
{
}

bool Mapa::cargarMapaDesdePNG(const QString& rutaMapa)
{
    qDebug() << "🔍 Intentando cargar mapa PNG desde:" << rutaMapa;

    mapaCompleto = QPixmap(rutaMapa);

    if(mapaCompleto.isNull()) {
        qDebug() << "❌ ERROR: No se pudo cargar el mapa PNG";
        qDebug() << "   Ruta:" << rutaMapa;
        return false;
    }

    qDebug() << "✅ MAPA PNG CARGADO EXITOSAMENTE";
    qDebug() << "   Tamaño del mapa:" << mapaCompleto.width() << "x" << mapaCompleto.height();

    procesarColisionesDesdePNG();

    posicionInicio = QPointF(mapaCompleto.width() / 2, mapaCompleto.height() / 2);
    qDebug() << "📍 Posición inicial:" << posicionInicio;

    return true;
}

void Mapa::procesarColisionesDesdePNG()
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
            capaColisiones[y][x] = !esPixelTransitable(color);
        }
    }

    qDebug() << "🎯 Capa de colisiones procesada:" << ancho << "x" << alto;
}

bool Mapa::esPixelTransitable(const QColor& color) const
{
    return !(color.red() == 0 && color.green() == 0 && color.blue() == 0 && color.alpha() == 255);

}

void Mapa::crearMapaBasico()
{
    mapaCompleto = QPixmap(1300, 730);
    mapaCompleto.fill(QColor(100, 100, 100));

    QPainter painter(&mapaCompleto);

    painter.setBrush(Qt::black);
    painter.setPen(Qt::NoPen);

    painter.drawRect(0, 0, 1300, 20);
    painter.drawRect(0, 710, 1300, 20);
    painter.drawRect(0, 0, 20, 730);
    painter.drawRect(1280, 0, 20, 730);

    painter.drawRect(300, 200, 100, 100);
    painter.drawRect(600, 400, 150, 80);
    painter.drawRect(800, 100, 80, 200);

    painter.end();

    procesarColisionesDesdePNG();

    posicionInicio = QPointF(100, 100);
    qDebug() << "🗺️  Mapa básico creado:" << mapaCompleto.width() << "x" << mapaCompleto.height();
}

void Mapa::dibujar(QPainter& painter, const QRectF& vista)
{
    if(mapaCompleto.isNull()) {
        painter.fillRect(vista, QColor(60, 60, 80));
        return;
    }

    painter.drawPixmap(vista, mapaCompleto, vista);
}

QPointF Mapa::getPosicionInicioJugador() const {
    return posicionInicio;
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

QVector<QRectF> Mapa::obtenerColisiones() const
{
    QVector<QRectF> colisiones;

    for(int y = 0; y < capaColisiones.size(); y++) {
        for(int x = 0; x < capaColisiones[y].size(); x++) {
            if(capaColisiones[y][x]) {
                colisiones.append(QRectF(x, y, 1, 1));
            }
        }
    }

    return colisiones;
}

QRectF Mapa::getLimitesMapa() const
{
    return QRectF(0, 0, mapaCompleto.width(), mapaCompleto.height());
}
