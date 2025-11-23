#include "entidad.h"

Entidad::Entidad() : vida(100.0f), velocidad(0.0f)
{
    posicion = QPointF(0, 0);
}

void Entidad::mover(const QPointF& direccion) {
    posicion += direccion * velocidad;
}

void Entidad::recibirDanio(float cantidad) {
    vida -= cantidad;
    if(vida < 0) vida = 0;
}

QRectF Entidad::getAreaColision() const {
    return QRectF(posicion.x() - 10, posicion.y() - 10, 20, 20);
}
