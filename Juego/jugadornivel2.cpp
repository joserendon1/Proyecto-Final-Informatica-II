#include "jugadornivel2.h"
#include "audiomanager.h"
#include <QDebug>

JugadorNivel2::JugadorNivel2()
{
    vida = 100.0f;
    velocidad = 0.0f;
    posicion = QPointF(512, 650); // Posición inicial abajo del área de juego
    ultimaDireccion = QPointF(0, -1);
    teclasPresionadas.resize(4, false);
}

JugadorNivel2::~JugadorNivel2()
{
    // No hay torres que limpiar en este nuevo diseño
}

const QList<Arma*>& JugadorNivel2::getArmas() const
{
    static QList<Arma*> armasVacio;
    return armasVacio;
}

void JugadorNivel2::actualizar(float deltaTime)
{
    // Procesar input para movimiento horizontal
    if (teclasPresionadas[0]) { // A - izquierda
        moverIzquierda();
    }
    if (teclasPresionadas[1]) { // D - derecha
        moverDerecha();
    }
}

void JugadorNivel2::activarArmas()
{
    // No hay armas en este nivel
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

bool JugadorNivel2::tieneArma(Arma::Tipo tipo) const
{
    Q_UNUSED(tipo);
    return false;
}

void JugadorNivel2::anadirArmaNueva(Arma::Tipo tipoArma)
{
    Q_UNUSED(tipoArma);
    // Implementación vacía - este nivel no usa armas
}

QRectF JugadorNivel2::getAreaColision() const
{
    return QRectF(posicion.x() - 20, posicion.y() - 20, 40, 40);
}
