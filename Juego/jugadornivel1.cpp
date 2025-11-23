#include "jugadornivel1.h"
#include "audiomanager.h"
#include <QtMath>
#include <QDebug>
#include <vector>

JugadorNivel1::JugadorNivel1()
{
    vida = 100.0f;
    velocidad = 2.0f;
    posicion = QPointF(400, 300);
    ultimaDireccion = QPointF(0, -1);

    // Inicializar vector de teclas (4 elementos para WASD)
    teclasPresionadas.resize(4, false);

    armas.append(new Arma(Arma::BALLESTA));

    for(Arma* arma : armas) {
        arma->setReferenciasJugador(&posicion, &ultimaDireccion);
    }
}

JugadorNivel1::~JugadorNivel1() {
    for(Arma* arma : armas) {
        delete arma;
    }
    armas.clear();
}

void JugadorNivel1::actualizar(float deltaTime) {
    velocidadMovimiento = QPointF(0, 0);

    if(teclasPresionadas[0]) {
        velocidadMovimiento.setY(-1);
        ultimaDireccion = QPointF(0, -1);
    }
    if(teclasPresionadas[1]) {
        velocidadMovimiento.setX(-1);
        ultimaDireccion = QPointF(-1, 0);
    }
    if(teclasPresionadas[2]) {
        velocidadMovimiento.setY(1);
        ultimaDireccion = QPointF(0, 1);
    }
    if(teclasPresionadas[3]) {
        velocidadMovimiento.setX(1);
        ultimaDireccion = QPointF(1, 0);
    }

    if(velocidadMovimiento.x() != 0 && velocidadMovimiento.y() != 0) {
        velocidadMovimiento /= qSqrt(2);
    }

    if(velocidadMovimiento.x() != 0 || velocidadMovimiento.y() != 0) {
        mover(velocidadMovimiento);
    }

    for(Arma* arma : armas) {
        arma->actualizar(deltaTime);
    }

    activarArmas();
}

void JugadorNivel1::activarArmas() {
    for(Arma* arma : armas) {
        if(arma->puedeAtacar()) {
            arma->activar(posicion, ultimaDireccion);

            if (arma->getTipo() == Arma::BALLESTA || arma->getTipo() == Arma::ARCO) {
                AudioManager::getInstance().playArrowShot();
            }
        }
    }
}

void JugadorNivel1::procesarInput(const std::vector<bool>& teclas) {
    for(size_t i = 0; i < 4 && i < teclas.size(); i++) {
        teclasPresionadas[i] = teclas[i];
    }
}

void JugadorNivel1::aplicarMejoraVida(float extra) {
    vida += extra;
}

void JugadorNivel1::aplicarMejoraDanio(float extra) {
    danioExtra += extra;
    for(Arma* arma : armas) {
        arma->setDanio(arma->getDanio() + extra);
    }
}

void JugadorNivel1::aplicarMejoraVelocidad(float extra) {
    velocidadExtra += extra;
    setVelocidad(2.5f + velocidadExtra);
}

void JugadorNivel1::ganarExperiencia(int exp)
{
    int expBase = exp;

    if(nivel > 5) {
        expBase += nivel * 2;
    }

    experiencia += expBase;

    if(expBase >= 25 || nivel % 2 == 0) {
        qDebug() << "📈 +" << expBase << "EXP - Total:" << experiencia
                 << "/" << getExperienciaParaSiguienteNivel();
    }

    if(experiencia >= getExperienciaParaSiguienteNivel()) {
        subirNivel();
    }
}

void JugadorNivel1::subirNivel()
{
    nivel++;
    experiencia = 0;

    vida += 20 + (nivel * 2);

    if(nivel % 3 == 0) {
        velocidad += 0.1f;
    }

    if(nivel % 5 == 0) {
        aplicarMejoraDanio(2.0f);
    }

    mejoraPendiente = true;

    qDebug() << "🎉 ¡Subiste al nivel" << nivel << "!";
    qDebug() << "📊 Próximo nivel en:" << getExperienciaParaSiguienteNivel() << "EXP";
}

int JugadorNivel1::getExperienciaParaSiguienteNivel() const {
    int base = 100;
    float factorDificultad = 1.8f;

    int expRequerida = base * qPow(nivel, factorDificultad);

    expRequerida = qMax(expRequerida, nivel * 150);

    if(nivel > 10) {
        expRequerida *= 0.9f;
    }

    return expRequerida;
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
    nuevaArma->setReferenciasJugador(&posicion, &ultimaDireccion);
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
