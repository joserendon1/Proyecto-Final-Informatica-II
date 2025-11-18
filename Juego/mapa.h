#ifndef MAPA_H
#define MAPA_H

#include <QObject>
#include <QPainter>
#include <QPointF>
#include <QVector>
#include <QRectF>
#include <QPixmap>
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QImage>

// Forward declaration para evitar includes circulares
class QWidget;

class Mapa : public QObject
{
    Q_OBJECT

public:
    Mapa(QObject *parent = nullptr);
    ~Mapa();

    bool cargarMapaDesdePNG(const QString& rutaMapa);
    void crearMapaBasico();
    void crearMapaGrande(const QSize& tamano);
    void escalarMapa(const QSize& nuevoTamano);

    void dibujar(QPainter& painter, const QRectF& vista);
    QPointF getPosicionInicioJugador() const;

    // Para colisiones
    bool esTransitable(int x, int y) const;
    QVector<QRectF> obtenerColisiones() const;
    QRectF getLimitesMapa() const;

    // Getters
    QSize getTamanoMapa() const { return mapaCompleto.size(); }
    QPixmap getMapaCompleto() const { return mapaCompleto; }

private:
    QPixmap mapaCompleto;
    QVector<QVector<bool>> capaColisiones;

    QPointF posicionInicio;

    void procesarColisionesDesdePNG();
    bool esPixelTransitable(const QColor& color) const;
};

#endif // MAPA_H
