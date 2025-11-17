#include "jugadornivel1.h"
#include <QtMath>
#include <QDebug>  // AGREGAR este include

JugadorNivel1::JugadorNivel1() :
    nivel(1),
    experiencia(0),
    mejoraPendiente(false),
    danioExtra(0),
    velocidadExtra(0)
{
    vida = 100.0f;
    velocidad = 2.5f;
    posicion = QPointF(400, 300);

    for(int i = 0; i < 4; i++) {
        teclasPresionadas[i] = false;
    }

    // SOLO UNA ARMA INICIAL - la espada
    armas.append(new Arma(Arma::ESPADA));
}

JugadorNivel1::~JugadorNivel1() {
    for(Arma* arma : armas) {
        delete arma;
    }
    armas.clear();
}

void JugadorNivel1::actualizar(float deltaTime) {
    velocidadMovimiento = QPointF(0, 0);

    if(teclasPresionadas[0]) velocidadMovimiento.setY(-1); // W
    if(teclasPresionadas[1]) velocidadMovimiento.setX(-1); // A
    if(teclasPresionadas[2]) velocidadMovimiento.setY(1);  // S
    if(teclasPresionadas[3]) velocidadMovimiento.setX(1);  // D

    // Normalizar movimiento diagonal
    if(velocidadMovimiento.x() != 0 && velocidadMovimiento.y() != 0) {
        velocidadMovimiento /= qSqrt(2);
    }

    if(velocidadMovimiento.x() != 0 || velocidadMovimiento.y() != 0) {
        mover(velocidadMovimiento);
    }

    // Limitar al área de juego
    if(posicion.x() < 0) posicion.setX(0);
    if(posicion.x() > 800) posicion.setX(800);
    if(posicion.y() < 0) posicion.setY(0);
    if(posicion.y() > 600) posicion.setY(600);

    // Actualizar armas con deltaTime
    for(Arma* arma : armas) {
        arma->actualizar(deltaTime);
    }

    // Activar armas si pueden atacar
    activarArmas();
}

void JugadorNivel1::activarArmas() {
    for(Arma* arma : armas) {
        if(arma->puedeAtacar()) {
            arma->activar(posicion);
        }
    }
}

void JugadorNivel1::procesarInput(bool teclas[]) {
    for(int i = 0; i < 4; i++) {
        teclasPresionadas[i] = teclas[i];
    }
}

void JugadorNivel1::aplicarMejoraVida(float extra) {
    vida += extra;
}

void JugadorNivel1::aplicarMejoraDanio(float extra) {
    danioExtra += extra;
    // Aplicar el daño extra a todas las armas
    for(Arma* arma : armas) {
        arma->setDanio(arma->getDanio() + extra);
    }
}

void JugadorNivel1::aplicarMejoraVelocidad(float extra) {
    velocidadExtra += extra;
    setVelocidad(2.5f + velocidadExtra); // Base 2.5 + extra
}

void JugadorNivel1::ganarExperiencia(int exp) {
    experiencia += exp;
    if(experiencia >= getExperienciaParaSiguienteNivel()) {
        subirNivel();
    }
}

void JugadorNivel1::subirNivel() {
    nivel++;
    experiencia = 0;
    vida += 25; // Curar parcialmente al subir de nivel
    mejoraPendiente = true;

    qDebug() << "¡Subiste al nivel" << nivel << "! Elige una mejora.";
}

// IMPLEMENTACIÓN del método (quitada del .h)
int JugadorNivel1::getExperienciaParaSiguienteNivel() const {
    return nivel * 150; // AUMENTADO de 100 a 150 (más difícil subir)
}

QRectF JugadorNivel1::getAreaColision() const {
    return QRectF(posicion.x() - 10, posicion.y() - 10, 20, 20);
}

void JugadorNivel1::anadirArmaNueva(Arma::Tipo tipoArma) {
    // Verificar si ya tiene el arma
    if (tieneArma(tipoArma)) {
        qDebug() << "Ya tienes esta arma!";
        return;
    }

    // Crear y agregar la nueva arma
    Arma* nuevaArma = new Arma(tipoArma);
    armas.append(nuevaArma);

    // Aplicar daño extra si ya hay mejoras de daño
    if (danioExtra > 0) {
        nuevaArma->setDanio(nuevaArma->getDanio() + danioExtra);
    }

    qDebug() << "¡Nueva arma añadida:" << tipoArma << "!";
}

bool JugadorNivel1::tieneArma(Arma::Tipo tipo) const {
    for (Arma* arma : armas) {
        if (arma->getTipo() == tipo) {
            return true;
        }
    }
    return false;
}
