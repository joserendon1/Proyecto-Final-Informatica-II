#include "jugadornivel2.h"
#include "audiomanager.h"
#include <QDebug>

JugadorNivel2::JugadorNivel2()
{
    vida = 200.0f;
    velocidad = 0.0f;
    posicion = QPointF(400, 300);
    posicionBase = posicion;
    ultimaDireccion = QPointF(0, -1);
    teclasPresionadas.resize(4, false);
}

JugadorNivel2::~JugadorNivel2()
{
    for(Torre* torre : torres) {
        delete torre;
    }
    torres.clear();
}

// Implementación corregida de getArmas()
const QList<Arma*>& JugadorNivel2::getArmas() const
{
    armasCache.clear();
    for(Torre* torre : torres) {
        armasCache.append(static_cast<Arma*>(torre));
    }
    return armasCache;
}

void JugadorNivel2::actualizar(float deltaTime)
{
    for(Torre* torre : torres) {
        torre->actualizar(deltaTime);
    }
    activarArmas();
}

void JugadorNivel2::activarArmas()
{
    for(Torre* torre : torres) {
        if(torre->puedeAtacar()) {
            torre->activar(torre->getPosicion(), QPointF(0, -1));
        }
    }
}

void JugadorNivel2::procesarInput(const std::vector<bool>& teclas)
{
    for(size_t i = 0; i < teclas.size() && i < 4; i++) {
        teclasPresionadas[i] = teclas[i];
    }
}

bool JugadorNivel2::construirTorre(Torre::Tipo tipo, const QPointF& posicion)
{
    Torre* nuevaTorre = new Torre(tipo);

    if(!gastarRecursos(nuevaTorre->getCosto())) {
        delete nuevaTorre;
        return false;
    }

    nuevaTorre->setPosicion(posicion);
    torres.append(nuevaTorre);

    qDebug() << "Torre construida:" << nuevaTorre->getNombre()
             << "en posición:" << posicion
             << "Recursos restantes:" << recursos;

    return true;
}

void JugadorNivel2::mejorarTorre(Torre* torre)
{
    if(!torre) return;

    int costoMejora = torre->getCostoMejora();
    if(gastarRecursos(costoMejora)) {
        torre->mejorar();
        qDebug() << "Torre mejorada. Recursos restantes:" << recursos;
    }
}

void JugadorNivel2::venderTorre(Torre* torre)
{
    if(!torre) return;

    int reembolso = torre->getCostoVenta();
    recursos += reembolso;
    torres.removeOne(torre);
    delete torre;

    qDebug() << "Torre vendida. Reembolso:" << reembolso
             << "Recursos totales:" << recursos;
}

Torre* JugadorNivel2::getTorreEnPosicion(const QPointF& posicion) const
{
    float radioDeteccion = 30.0f;

    for(Torre* torre : torres) {
        QPointF distancia = torre->getPosicion() - posicion;
        float magnitud = qSqrt(distancia.x() * distancia.x() + distancia.y() * distancia.y());

        if(magnitud <= radioDeteccion) {
            return torre;
        }
    }

    return nullptr;
}

bool JugadorNivel2::gastarRecursos(int cantidad)
{
    if(recursos >= cantidad) {
        recursos -= cantidad;
        return true;
    }
    return false;
}

bool JugadorNivel2::tieneArma(Arma::Tipo tipo) const
{
    Q_UNUSED(tipo);
    return false;
}

void JugadorNivel2::anadirArmaNueva(Arma::Tipo tipoArma)
{
    Q_UNUSED(tipoArma);
}

QRectF JugadorNivel2::getAreaColision() const
{
    return QRectF(posicion.x() - 40, posicion.y() - 40, 80, 80);
}
