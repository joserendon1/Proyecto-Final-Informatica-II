#include "jugadornivel1.h"
#include <QtMath>

JugadorNivel1::JugadorNivel1()
    : nivel(1), experiencia(0), mejoraPendiente(false), danioExtra(0), velocidadExtra(0)
{
    vida = 100.0f;
    velocidad = 5.0f;
    posicion = QPointF(400, 300);

    for(int i = 0; i < 4; i++) {
        teclasPresionadas[i] = false;
    }

    armas.append(new Arma(Arma::ESPADA));
    armas.append(new Arma(Arma::BALLESTA));
}

JugadorNivel1::~JugadorNivel1() {
    for(Arma* arma : armas) {
        delete arma;
    }
    armas.clear();
}

void JugadorNivel1::mover(const QPointF& direccion) {
    float velocidadTotal = velocidad + velocidadExtra;
    posicion += direccion * velocidadTotal;

    // Limitar al área de juego
    if(posicion.x() < 0) posicion.setX(0);
    if(posicion.x() > 800) posicion.setX(800);
    if(posicion.y() < 0) posicion.setY(0);
    if(posicion.y() > 600) posicion.setY(600);
}

void JugadorNivel1::actualizar() {
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

    // Actualizar armas
    activarArmas();
}

void JugadorNivel1::activarArmas() {
    for(Arma* arma : armas) {
        arma->actualizar();

        // Las armas se activan automáticamente cuando pueden
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

void JugadorNivel1::recibirDanio(float cantidad) {
    vida -= cantidad;
    if(vida < 0) vida = 0;
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
    mejoraPendiente = true; // Indicar que hay mejora pendiente

    qDebug() << "¡Subiste al nivel" << nivel << "! Elige una mejora.";
}
