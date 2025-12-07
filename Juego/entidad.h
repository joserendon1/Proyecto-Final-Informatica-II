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
    virtual void mover(const QPointF& direccion);
    virtual void recibirDanio(float cantidad);

    QPointF getPosicion() const { return posicion; }
    void setPosicion(const QPointF& nuevaPosicion) { posicion = nuevaPosicion; }

    float getVida() const { return vida; }
    void setVida(float nuevaVida) { vida = nuevaVida; }

    float getVelocidad() const { return velocidad; }
    void setVelocidad(float nuevaVelocidad) { velocidad = nuevaVelocidad; }

    bool estaViva() const { return vida > 0; }

    virtual QRectF getAreaColision() const;

protected:
    QPointF posicion;
    float vida;
    float velocidad;
};

#endif // ENTIDAD_H
