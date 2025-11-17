#include "enemigo.h"
#include <QtMath>

Enemigo::Enemigo()
    : tipo(1), experienciaQueDa(10)
{
    vida = 30.0f;
    velocidad = 1.2f;  // REDUCIDO de 2.0f a 1.2f (más lento)
    posicion = QPointF(0, 0);
}

void Enemigo::actualizar(float deltaTime) {
    Q_UNUSED(deltaTime);
}

void Enemigo::seguirJugador(const QPointF& posicionJugador) {
    QPointF direccion = posicionJugador - posicion;

    // Normalizar la dirección
    float magnitud = qSqrt(direccion.x() * direccion.x() + direccion.y() * direccion.y());
    if(magnitud > 0) {
        direccion /= magnitud;
    }

    mover(direccion);
}

void Enemigo::recibirDanio(float cantidad) {
    vida -= cantidad;
    if(vida < 0) vida = 0;
}

QRectF Enemigo::getAreaColision() const {
    return QRectF(posicion.x() - 8, posicion.y() - 8, 16, 16);
}
