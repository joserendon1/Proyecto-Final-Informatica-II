#include "enemigo.h"
#include <QtMath>

Enemigo::Enemigo(int tipoEnemigo) : tipo(tipoEnemigo)
{
    if(tipo == 1) {
        vida = 25.0f;
        velocidad = 1.8f;
        experienciaQueDa = 15;
    } else {
        vida = 70.0f;
        velocidad = 1.1f;
        experienciaQueDa = 35;
    }
    posicion = QPointF(0, 0);
}

void Enemigo::actualizar(float deltaTime)
{
    Q_UNUSED(deltaTime);
}

void Enemigo::seguirJugador(const QPointF& posicionJugador)
{
    QPointF direccion = posicionJugador - posicion;
    float magnitud = qSqrt(direccion.x() * direccion.x() + direccion.y() * direccion.y());

    if(magnitud > 0) {
        direccion /= magnitud;
        mover(direccion);
    }
}

void Enemigo::recibirDanio(float cantidad)
{
    vida -= cantidad;
    if(vida < 0) vida = 0;
}

QRectF Enemigo::getAreaColision() const
{
    if(tipo == 1) {
        return QRectF(posicion.x() - 6, posicion.y() - 6, 12, 12);
    } else {
        return QRectF(posicion.x() - 8, posicion.y() - 8, 16, 16);
    }
}
