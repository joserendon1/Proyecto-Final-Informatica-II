#include "mapa.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QRandomGenerator>
#include <QImage>
#include <QWidget>  // NUEVO: Include necesario para QWidget
#include <QPainter> // NUEVO: Include para QPainter

Mapa::Mapa(QObject *parent) : QObject(parent)
{
    posicionInicio = QPointF(100, 100);
}

Mapa::~Mapa()
{
    // Limpiar capa de colisiones
    capaColisiones.clear();
}

bool Mapa::cargarMapaDesdePNG(const QString& rutaMapa)
{
    qDebug() << "🔍 Intentando cargar mapa PNG desde:" << rutaMapa;

    mapaCompleto = QPixmap(rutaMapa);

    if(mapaCompleto.isNull()) {
        qDebug() << "❌ ERROR: No se pudo cargar el mapa PNG";
        return false;
    }

    qDebug() << "✅ MAPA PNG CARGADO EXITOSAMENTE";
    qDebug() << "   Tamaño:" << mapaCompleto.width() << "x" << mapaCompleto.height();

    procesarColisionesDesdePNG();
    posicionInicio = QPointF(mapaCompleto.width() / 2, mapaCompleto.height() / 2);

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
    // Negro sólido = no transitable
    return !(color.red() == 0 && color.green() == 0 && color.blue() == 0 && color.alpha() == 255);
}

void Mapa::crearMapaGrande(const QSize& tamano)
{
    mapaCompleto = QPixmap(tamano);

    // FONDO MÁS CLARO Y CON MEJOR CONTRASTE
    mapaCompleto.fill(QColor(120, 120, 140)); // Gris claro

    QPainter painter(&mapaCompleto);
    painter.setRenderHint(QPainter::Antialiasing);

    // PATRÓN DE CUADRÍCULA MÁS VISIBLE
    painter.setPen(QPen(QColor(100, 100, 120), 2));
    int gridSize = 200;
    for(int x = 0; x < tamano.width(); x += gridSize) {
        painter.drawLine(x, 0, x, tamano.height());
    }
    for(int y = 0; y < tamano.height(); y += gridSize) {
        painter.drawLine(0, y, tamano.width(), y);
    }

    // BORDES GRUESOS Y VISIBLES
    painter.setBrush(QBrush(QColor(30, 30, 50)));
    painter.setPen(Qt::NoPen);

    int grosorBorde = 100;
    int ancho = tamano.width();
    int alto = tamano.height();

    painter.drawRect(0, 0, ancho, grosorBorde);
    painter.drawRect(0, alto - grosorBorde, ancho, grosorBorde);
    painter.drawRect(0, 0, grosorBorde, alto);
    painter.drawRect(ancho - grosorBorde, 0, grosorBorde, alto);

    // OBSTÁCULOS MÁS VISIBLES Y CONTRASTADOS
    painter.setBrush(QBrush(QColor(40, 40, 60)));
    painter.setPen(QPen(QColor(20, 20, 40), 3));

    // Obstáculo central grande (para referencia)
    painter.drawRect(ancho/2 - 150, alto/2 - 150, 300, 300);

    // Obstáculos en las esquinas
    painter.drawRect(ancho * 0.1, alto * 0.1, 200, 200);
    painter.drawRect(ancho * 0.7, alto * 0.1, 200, 200);
    painter.drawRect(ancho * 0.1, alto * 0.7, 200, 200);
    painter.drawRect(ancho * 0.7, alto * 0.7, 200, 200);

    // Algunos obstáculos aleatorios
    for(int i = 0; i < 10; i++) {
        int x = QRandomGenerator::global()->bounded(ancho - 100);
        int y = QRandomGenerator::global()->bounded(alto - 100);
        int w = 50 + QRandomGenerator::global()->bounded(100);
        int h = 50 + QRandomGenerator::global()->bounded(100);
        painter.drawRect(x, y, w, h);
    }

    painter.end();

    // DEBUG DEL MAPA (CORREGIDO - SIN .format())
    qDebug() << "🗺️ Mapa grande creado - Tamaño:" << mapaCompleto.size();
    qDebug() << "   ¿Mapa es nulo?" << mapaCompleto.isNull();

    procesarColisionesDesdePNG();

    // Posición inicial en el centro
    posicionInicio = QPointF(ancho / 2, alto / 2);

    qDebug() << "📍 Posición inicial del jugador:" << posicionInicio;
}

void Mapa::escalarMapa(const QSize& nuevoTamano)
{
    if(mapaCompleto.isNull()) return;

    qDebug() << "🔄 Escalando mapa de" << mapaCompleto.size() << "a" << nuevoTamano;

    // Escalar el pixmap
    QPixmap mapaEscalado = mapaCompleto.scaled(nuevoTamano, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    mapaCompleto = mapaEscalado;

    // Reprocesar colisiones para el nuevo tamaño
    procesarColisionesDesdePNG();

    // Actualizar posición inicial al centro del nuevo mapa
    posicionInicio = QPointF(nuevoTamano.width() / 2, nuevoTamano.height() / 2);

    qDebug() << "✅ Mapa escalado a:" << nuevoTamano;
}

void Mapa::crearMapaBasico()
{
    // SOLUCIÓN: Crear mapa del tamaño estándar que coincida con la ventana
    QSize tamano(1350, 800); // Mismo tamaño que MainWindow

    mapaCompleto = QPixmap(tamano);
    mapaCompleto.fill(QColor(100, 100, 100));

    QPainter painter(&mapaCompleto);
    painter.setBrush(Qt::black);
    painter.setPen(Qt::NoPen);

    // Bordes del tamaño exacto
    int ancho = tamano.width();
    int alto = tamano.height();
    int grosorBorde = 20;

    painter.drawRect(0, 0, ancho, grosorBorde); // Arriba
    painter.drawRect(0, alto - grosorBorde, ancho, grosorBorde); // Abajo
    painter.drawRect(0, 0, grosorBorde, alto); // Izquierda
    painter.drawRect(ancho - grosorBorde, 0, grosorBorde, alto); // Derecha

    // Obstáculos centrados en el mapa
    painter.drawRect(ancho * 0.25, alto * 0.3, ancho * 0.1, alto * 0.15);
    painter.drawRect(ancho * 0.5, alto * 0.5, ancho * 0.12, alto * 0.1);
    painter.drawRect(ancho * 0.7, alto * 0.2, ancho * 0.08, alto * 0.25);

    painter.end();
    procesarColisionesDesdePNG();

    // Posición inicial en el centro
    posicionInicio = QPointF(ancho / 2, alto / 2);

    qDebug() << "🗺️ Mapa básico creado:" << mapaCompleto.width() << "x" << mapaCompleto.height();
    qDebug() << "📍 Posición inicial:" << posicionInicio;
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

QPointF Mapa::getPosicionInicioJugador() const {
    return posicionInicio;
}
