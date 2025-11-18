#include "jugadornivel1.h"
#include <QtMath>
#include <QDebug>  // AGREGAR este include

JugadorNivel1::JugadorNivel1() :
    nivel(1), experiencia(0), mejoraPendiente(false),
    danioExtra(0), velocidadExtra(0),
    ultimaDireccion(0, -1)
{
    vida = 100.0f;
    velocidad = 2.0f;
    posicion = QPointF(400, 300);

    for(int i = 0; i < 4; i++) {
        teclasPresionadas[i] = false;
    }

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

    // ACTUALIZAR: Cada tecla actualiza la última dirección
    if(teclasPresionadas[0]) {
        velocidadMovimiento.setY(-1);
        ultimaDireccion = QPointF(0, -1); // W = Arriba
    }
    if(teclasPresionadas[1]) {
        velocidadMovimiento.setX(-1);
        ultimaDireccion = QPointF(-1, 0); // A = Izquierda
    }
    if(teclasPresionadas[2]) {
        velocidadMovimiento.setY(1);
        ultimaDireccion = QPointF(0, 1);  // S = Abajo
    }
    if(teclasPresionadas[3]) {
        velocidadMovimiento.setX(1);
        ultimaDireccion = QPointF(1, 0);  // D = Derecha
    }

    if(velocidadMovimiento.x() != 0 && velocidadMovimiento.y() != 0) {
        velocidadMovimiento /= qSqrt(2);
    }

    if(velocidadMovimiento.x() != 0 || velocidadMovimiento.y() != 0) {
        mover(velocidadMovimiento);
    }

    // Limitar al área de juego
    if(posicion.x() < 0) posicion.setX(0);
    if(posicion.x() > 1300) posicion.setX(1300);
    if(posicion.y() < 0) posicion.setY(0);
    if(posicion.y() > 730) posicion.setY(730);

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
            arma->activar(posicion, ultimaDireccion);
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

int JugadorNivel1::getExperienciaParaSiguienteNivel() const {
    return nivel * 200;
}

QRectF JugadorNivel1::getAreaColision() const {
    return QRectF(posicion.x() - 10, posicion.y() - 10, 20, 20);
}

void JugadorNivel1::anadirArmaNueva(Arma::Tipo tipoArma) {
    if (tieneArma(tipoArma)) {
        qDebug() << "Ya tienes esta arma! Mejorando la existente...";
        for (Arma* arma : armas) {
            if (arma->getTipo() == tipoArma) {
                arma->subirNivel();
                qDebug() << "¡" << arma->getNombre() << " mejorada a nivel" << arma->getNivel() << "!";
                break;
            }
        }
        return;
    }

    Arma* nuevaArma = new Arma(tipoArma);
    armas.append(nuevaArma);

    qDebug() << "¡Nueva arma añadida:" << nuevaArma->getNombre() << "!";
}

bool JugadorNivel1::tieneArma(Arma::Tipo tipo) const {
    for (Arma* arma : armas) {
        if (arma->getTipo() == tipo) {
            return true;
        }
    }
    return false;
}
