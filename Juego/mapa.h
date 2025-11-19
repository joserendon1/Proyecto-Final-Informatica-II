#ifndef MAPA_H
#define MAPA_H

#include <QObject>
#include <QPainter>
#include <QPointF>
#include <QVector>
#include <QRectF>
#include <QPixmap>

class Mapa : public QObject
{
    Q_OBJECT

public:
    Mapa(QObject *parent = nullptr);
    ~Mapa();

    void crearMapaGrande(const QSize& tamano);
    void dibujar(QPainter& painter, const QRectF& vista);
    QPointF getPosicionInicioJugador() const;
    void procesarColisionesDesdeMapa();
    bool esTransitable(int x, int y) const;
    QRectF getLimitesMapa() const;
    QSize getTamanoMapa() const { return mapaCompleto.size(); }
    QPixmap getMapaCompleto() const { return mapaCompleto; }

private:
    QPixmap mapaCompleto;
    QVector<QVector<bool>> capaColisiones;
    QPointF posicionInicio;

};

#endif // MAPA_H
