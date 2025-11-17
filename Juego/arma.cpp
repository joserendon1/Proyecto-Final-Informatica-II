#include "arma.h"
#include <QTimer>
#include <QtMath>
#include <QRandomGenerator>

Arma::Arma(Tipo tipoArma)
    : tipo(tipoArma), listaParaAtacar(true)
{
    timerCooldown = new QTimer();
    timerCooldown->setSingleShot(true);
    connect(timerCooldown, &QTimer::timeout, this, &Arma::resetCooldown);

    // Configurar según el tipo de arma
    switch(tipo) {
    case ESPADA:
        danio = 25.0f;
        cooldown = 800; // 0.8 segundos
        break;
    case BALLESTA:
        danio = 15.0f;
        cooldown = 1200; // 1.2 segundos
        break;
    case ACEITE:
        danio = 35.0f;
        cooldown = 2000; // 2 segundos
        break;
    }
}

Arma::~Arma() {
    if(timerCooldown) {
        timerCooldown->stop();
        delete timerCooldown;
    }
}

void Arma::activar(const QPointF& posicionJugador)
{
    if(!listaParaAtacar) return;

    areasAtaque.clear();
    proyectiles.clear();
    direccionesProyectiles.clear();

    switch(tipo) {
    case ESPADA:
        crearAtaqueEspada(posicionJugador);
        break;
    case BALLESTA:
        crearAtaqueBallesta(posicionJugador);
        break;
    case ACEITE:
        // Para futura implementación
        break;
    }

    listaParaAtacar = false;
    timerCooldown->start(cooldown);
}

void Arma::actualizar()
{
    if(tipo == BALLESTA) {
        actualizarProyectiles();
    }
}

void Arma::crearAtaqueEspada(const QPointF& posicion)
{
    // Ataque en 4 direcciones alrededor del jugador
    areasAtaque.append(QRectF(posicion.x() - 40, posicion.y() - 60, 80, 40)); // Arriba
    areasAtaque.append(QRectF(posicion.x() + 20, posicion.y() - 40, 40, 80)); // Derecha
    areasAtaque.append(QRectF(posicion.x() - 40, posicion.y() + 20, 80, 40)); // Abajo
    areasAtaque.append(QRectF(posicion.x() - 60, posicion.y() - 40, 40, 80)); // Izquierda
}

void Arma::crearAtaqueBallesta(const QPointF& posicion)
{
    // Crear proyectil hacia arriba
    proyectiles.append(posicion);
    direccionesProyectiles.append(QPointF(0, -1));
}

void Arma::actualizarProyectiles()
{
    // Mover proyectiles existentes
    for(int i = 0; i < proyectiles.size(); i++) {
        proyectiles[i] += direccionesProyectiles[i] * 8.0f; // Velocidad del proyectil
    }

    // Eliminar proyectiles que salgan de pantalla
    for(int i = proyectiles.size() - 1; i >= 0; i--) {
        if(proyectiles[i].y() < -50 ||
            proyectiles[i].x() < -50 ||
            proyectiles[i].x() > 850 ||
            proyectiles[i].y() > 650) {
            proyectiles.removeAt(i);
            direccionesProyectiles.removeAt(i);
        }
    }
}

QList<QRectF> Arma::getAreasAtaque() const
{
    QList<QRectF> todasLasAreas = areasAtaque;

    // Agregar áreas de proyectiles para ballesta
    if(tipo == BALLESTA) {
        for(const QPointF& proyectil : proyectiles) {
            todasLasAreas.append(QRectF(proyectil.x() - 5, proyectil.y() - 5, 10, 10));
        }
    }

    return todasLasAreas;
}

void Arma::resetCooldown()
{
    listaParaAtacar = true;
    // Limpiar áreas de ataque de espada (los proyectiles se manejan aparte)
    if(tipo == ESPADA) {
        const_cast<QList<QRectF>&>(areasAtaque).clear();
    }
}
