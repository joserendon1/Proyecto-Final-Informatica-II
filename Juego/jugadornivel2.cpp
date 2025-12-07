#include "jugadornivel2.h"
#include "arma.h"
#include <QRectF>
#include <algorithm>

JugadorNivel2::JugadorNivel2() : JugadorBase()
{
    posicion = QPointF(400, 500);
    velocidad = 3.0f;
    teclasPresionadas.resize(4, false);
    seEstaMoviendo = false;
    direccionActual = 0;
    vida = 100.0f;
}

void JugadorNivel2::actualizar(float deltaTime)
{
    seEstaMoviendo = false;
    direccionActual = 0;

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
}

void JugadorNivel2::procesarInput(const std::vector<bool>& teclas)
{
    for(size_t i = 0; i < teclas.size() && i < 4; i++) {
        teclasPresionadas[i] = teclas[i];
    }
}

void JugadorNivel2::moverDerecha()
{
    float nuevaX = posicion.x() + velocidad;
    if (nuevaX <= limiteDerecho) {
        posicion.setX(nuevaX);
        ultimaDireccion = QPointF(1, 0);
    }
}

void JugadorNivel2::moverIzquierda()
{
    float nuevaX = posicion.x() - velocidad;
    if (nuevaX >= limiteIzquierdo) {
        posicion.setX(nuevaX);
        ultimaDireccion = QPointF(-1, 0);
    }
}

void JugadorNivel2::resetear()
{
    posicion = QPointF(400, 520);
    vida = 100.0f;
    std::fill(teclasPresionadas.begin(), teclasPresionadas.end(), false);
    seEstaMoviendo = false;
    direccionActual = 0;
}

QRectF JugadorNivel2::getAreaColision() const
{
    return QRectF(posicion.x() - 15, posicion.y() - 15, 30, 30);
}


void JugadorNivel2::activarArmas()
{

}

const QList<Arma*>& JugadorNivel2::getArmas() const
{
    return armas;
}

bool JugadorNivel2::tieneArma(Arma::Tipo tipo) const
{
    for (Arma* arma : armas) {
        if (arma && arma->getTipo() == tipo) {
            return true;
        }
    }
    return false;
}

void JugadorNivel2::anadirArmaNueva(Arma::Tipo tipoArma)
{

    Arma* nuevaArma = new Arma(tipoArma);
    armas.append(nuevaArma);
}
