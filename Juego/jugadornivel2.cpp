#include "jugadornivel2.h"
#include <QRectF>

JugadorNivel2::JugadorNivel2()
{
    posicion = QPointF(400, 500);
    teclasPresionadas.resize(4, false);
    seEstaMoviendo = false;
    direccionActual = 0;
}

void JugadorNivel2::actualizar(float deltaTime)
{
    Q_UNUSED(deltaTime);

    // Resetear estado de movimiento
    seEstaMoviendo = false;
    direccionActual = 0;

    // Verificar teclas presionadas
    if (teclasPresionadas[0]) {
        moverIzquierda();
        seEstaMoviendo = true;
        direccionActual = -1;
    }
    if (teclasPresionadas[1]) {
        moverDerecha();
        seEstaMoviendo = true;
        direccionActual = 1;
    }

    // Si ambas teclas están presionadas, mantener la última dirección
    if (teclasPresionadas[0] && teclasPresionadas[1]) {
        // Mantener la dirección anterior o elegir una por defecto
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

void JugadorNivel2::resetear()
{
    posicion = QPointF(400, 520);  // Ajustado de (512, 650)
    vida = 100.0f;
    teclasPresionadas.clear();
    teclasPresionadas.resize(4, false);
    seEstaMoviendo = false;
    direccionActual = 0;
}

QRectF JugadorNivel2::getAreaColision() const
{
    // Área de colisión un poco más pequeña que el sprite para ser más permisivo
    return QRectF(posicion.x() - 15, posicion.y() - 15, 30, 30);
}
