#include "enemigo.h"
#include <QtMath>

Enemigo::Enemigo(int tipoEnemigo) : tipo(tipoEnemigo)
{
    if(tipo == 1) {
        // ENEMIGO DÉBIL - MÁS DÉBIL PERO MÁS NUMEROSO
        vida = 25.0f;  // REDUCIDO de 30
        velocidad = 1.8f; // AUMENTADO de 1.5
        experienciaQueDa = 15; // AUMENTADO de 10
    } else {
        // ENEMIGO FUERTE - MÁS PELIGROSO
        vida = 70.0f;  // AUMENTADO de 60
        velocidad = 1.1f; // AUMENTADO ligeramente de 1.0
        experienciaQueDa = 35; // AUMENTADO de 25
    }
    posicion = QPointF(0, 0);
}

void Enemigo::actualizar(float deltaTime)
{
    Q_UNUSED(deltaTime);
    // Lógica específica de enemigo puede ir aquí
}

void Enemigo::seguirJugador(const QPointF& posicionJugador)
{
    QPointF direccion = posicionJugador - posicion;
    float magnitud = qSqrt(direccion.x() * direccion.x() + direccion.y() * direccion.y());

    if(magnitud > 0) {
        direccion /= magnitud;
    }

    mover(direccion);
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
