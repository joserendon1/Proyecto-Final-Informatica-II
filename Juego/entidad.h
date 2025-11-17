#ifndef ENTIDAD_H
#define ENTIDAD_H

#include <QPointF>
#include <QRectF>

class Entidad
{
public:
    Entidad();
    virtual ~Entidad() = default;

    virtual void actualizar(float deltaTime) = 0;
    virtual QRectF getAreaColision() const = 0;

    virtual void mover(const QPointF& direccion);
    virtual void recibirDanio(float cantidad);

    QPointF getPosicion() const { return posicion; }
    void setPosicion(const QPointF& newPosicion) { posicion = newPosicion; }
    float getVida() const { return vida; }
    void setVida(float newVida) { vida = newVida; }
    bool estaViva() const { return vida > 0; }
    float getVelocidad() const { return velocidad; }
    void setVelocidad(float newVelocidad) { velocidad = newVelocidad; }

protected:
    QPointF posicion;
    float vida;
    float velocidad;
};

#endif // ENTIDAD_H
