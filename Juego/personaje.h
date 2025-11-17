#ifndef PERSONAJE_H
#define PERSONAJE_H

#include <QPointF>
#include <QRectF>

class Personaje
{
public:
    Personaje();
    virtual ~Personaje() = default;

    // Métodos virtuales puros
    virtual void mover(const QPointF& direccion) = 0;
    virtual void actualizar() = 0;

    // Getters y setters
    QPointF getPosicion() const { return posicion; }
    void setPosicion(const QPointF& newPosicion) { posicion = newPosicion; }

    float getVida() const { return vida; }
    void setVida(float newVida) { vida = newVida; }

    bool estaVivo() const { return vida > 0; }

    QRectF getAreaColision() const { return QRectF(posicion.x() - 10, posicion.y() - 10, 20, 20); }

protected:
    QPointF posicion;
    float vida;
    float velocidad;
};

#endif // PERSONAJE_H
