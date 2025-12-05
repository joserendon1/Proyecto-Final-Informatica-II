#include "jugadornivel2.h"
#include <QRectF>

JugadorNivel2::JugadorNivel2()
{
    posicion = QPointF(512, 650);
    teclasPresionadas.resize(4, false);
}

void JugadorNivel2::actualizar(float deltaTime)
{
    Q_UNUSED(deltaTime);

    if (teclasPresionadas[0]) {
        moverIzquierda();
    }
    if (teclasPresionadas[1]) {
        moverDerecha();
    }
}

void JugadorNivel2::procesarInput(const std::vector<bool>& teclas)
{
    for(size_t i = 0; i < teclas.size() && i < 4; i++) {
        teclasPresionadas[i] = teclas[i];
    }
}

void JugadorNivel2::moverDerecha()
{
    float nuevaX = posicion.x() + velocidadMovimientoHorizontal;
    if (nuevaX <= limiteDerecho) {
        posicion.setX(nuevaX);
    }
}

void JugadorNivel2::moverIzquierda()
{
    float nuevaX = posicion.x() - velocidadMovimientoHorizontal;
    if (nuevaX >= limiteIzquierdo) {
        posicion.setX(nuevaX);
    }
}

QRectF JugadorNivel2::getAreaColision() const
{
    return QRectF(posicion.x() - 20, posicion.y() - 20, 40, 40);
}
