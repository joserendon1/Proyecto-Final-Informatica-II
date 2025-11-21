#ifndef MAPA_H
#define MAPA_H

#include <QObject>
#include <QPainter>
#include <QPointF>
#include <QVector>
#include <QRectF>
#include <QPixmap>
#include <QList>
#include <QTimer>

class Mapa : public QObject
{
    Q_OBJECT

public:
    struct ElementoMapa {
        QPixmap sprite;
        QPointF posicion;
        QRectF areaColision;
        bool esDecoracion;

        // NUEVO: Para animaciones
        bool esAnimado;
        int frameActual;
        int totalFrames;
        int frameWidth;
        int frameHeight;
        float tiempoAcumulado;
        float tiempoPorFrame;
    };

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

    // NUEVO: Para actualizar animaciones
    void actualizarAnimaciones(float deltaTime);

private:
    QPixmap mapaCompleto;
    QVector<QVector<bool>> capaColisiones;
    QPointF posicionInicio;

    // Sprites para el mapa
    QPixmap spriteSuelo;
    QPixmap spriteHouse;
    QPixmap spriteTower;
    QPixmap spriteRoca;

    // Spritesheets para decoración animada
    QPixmap spriteTreeSheet;
    QPixmap spriteBushSheet;

    // Elementos del mapa
    QList<ElementoMapa> elementosMapa;

    void cargarSpritesMapa();
    void dibujarElementosMapa(QPainter& painter, const QRectF& vista);
    QPixmap obtenerFrameAnimado(const ElementoMapa& elemento) const;

};

#endif // MAPA_H
