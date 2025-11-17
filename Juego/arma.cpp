#include "arma.h"
#include <QtMath>
#include <QRandomGenerator>

Arma::Arma(Tipo tipoArma) : tipo(tipoArma), tiempoCooldownRestante(0)
{
    switch(tipo) {
    case ESPADA:
        danio = 25.0f;
        cooldown = 1000;  // AUMENTADO de 800 a 1000 ms (más lento)
        break;
    case BALLESTA:
        danio = 15.0f;
        cooldown = 1500;  // AUMENTADO de 1200 a 1500 ms
        break;
    case ACEITE:
        danio = 35.0f;
        cooldown = 2500;  // AUMENTADO de 2000 a 2500 ms
        break;
    case ARCO:
        danio = 20.0f;
        cooldown = 1200;  // AUMENTADO de 1000 a 1200 ms
        break;
    case LANZA:
        danio = 30.0f;
        cooldown = 1800;  // AUMENTADO de 1500 a 1800 ms
        break;
    case ESCUDO:
        danio = 10.0f;
        cooldown = 800;   // AUMENTADO de 500 a 800 ms
        break;
    }
}

Arma::~Arma()
{
}

void Arma::activar(const QPointF& posicion, const QPointF& direccion)
{
    if(!puedeAtacar()) return;

    limpiarAtaques();

    switch(tipo) {
    case ESPADA:
        crearAtaqueEspada(posicion);
        break;
    case BALLESTA:
        crearAtaqueBallesta(posicion, direccion);
        break;
    case ACEITE:
        areasAtaque.append(QRectF(posicion.x() - 50, posicion.y() - 50, 100, 100));
        break;
    case ARCO:
        crearAtaqueBallesta(posicion, QPointF(0, -1));
        crearAtaqueBallesta(posicion, QPointF(1, 0));
        crearAtaqueBallesta(posicion, QPointF(0, 1));
        crearAtaqueBallesta(posicion, QPointF(-1, 0));
        break;
    case LANZA:
        areasAtaque.append(QRectF(posicion.x() - 5, posicion.y() - 5, 100, 10));
        break;
    case ESCUDO:
        areasAtaque.append(QRectF(posicion.x() - 30, posicion.y() - 30, 60, 60));
        break;
    }

    tiempoCooldownRestante = cooldown;
}

void Arma::actualizar(float deltaTime)
{
    if(tiempoCooldownRestante > 0) {
        tiempoCooldownRestante -= deltaTime;
        if(tiempoCooldownRestante < 0) tiempoCooldownRestante = 0;
    }

    if(tipo == BALLESTA || tipo == ARCO) {
        actualizarProyectiles(deltaTime);
    }

    if(tipo == ESPADA || tipo == ACEITE || tipo == LANZA || tipo == ESCUDO) {
        if(tiempoCooldownRestante < cooldown * 0.3f) {
            limpiarAtaques();
        }
    }
}

void Arma::crearAtaqueEspada(const QPointF& posicion)
{
    areasAtaque.append(QRectF(posicion.x() - 40, posicion.y() - 60, 80, 40));
    areasAtaque.append(QRectF(posicion.x() + 20, posicion.y() - 40, 40, 80));
    areasAtaque.append(QRectF(posicion.x() - 40, posicion.y() + 20, 80, 40));
    areasAtaque.append(QRectF(posicion.x() - 60, posicion.y() - 40, 40, 80));
}

void Arma::crearAtaqueBallesta(const QPointF& posicion, const QPointF& direccion)
{
    proyectiles.append(posicion);
    direccionesProyectiles.append(direccion);
    tiemposVidaProyectiles.append(2000.0f);
}

void Arma::actualizarProyectiles(float deltaTime)
{
    for(int i = 0; i < proyectiles.size(); i++) {
        // REDUCIDO de 0.3f a 0.15f (más lento)
        proyectiles[i] += direccionesProyectiles[i] * 0.15f * deltaTime;
        tiemposVidaProyectiles[i] -= deltaTime;
    }

    for(int i = proyectiles.size() - 1; i >= 0; i--) {
        if(tiemposVidaProyectiles[i] <= 0) {
            proyectiles.removeAt(i);
            direccionesProyectiles.removeAt(i);
            tiemposVidaProyectiles.removeAt(i);
        }
    }
}

QList<QRectF> Arma::getAreasAtaque() const
{
    QList<QRectF> todasLasAreas = areasAtaque;

    if(tipo == BALLESTA || tipo == ARCO) {
        for(const QPointF& proyectil : proyectiles) {
            todasLasAreas.append(QRectF(proyectil.x() - 5, proyectil.y() - 5, 10, 10));
        }
    }

    return todasLasAreas;
}

void Arma::limpiarAtaques()
{
    areasAtaque.clear();
}
