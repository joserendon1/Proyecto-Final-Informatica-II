#include "torre.h"
#include "enemigo.h"
#include <QtMath>
#include <QRandomGenerator>
#include <QDebug>

Torre::Torre(Arma::Tipo tipo) : Arma(tipo), nivel(1)
{
    // Configurar según el tipo de torre
    switch(tipo) {
    case Arma::ARCO:
        nombre = "Torre de Arco";
        danioBase = 8.0f;
        cooldownBase = 1200;
        costo = 40;
        rango = 180.0f;
        color = QColor(160, 82, 45);
        break;
    case Arma::BALLESTA:
        nombre = "Torre de Ballesta";
        danioBase = 15.0f;
        cooldownBase = 1800;
        costo = 60;
        rango = 220.0f;
        color = QColor(139, 69, 19);
        break;
    case Arma::CATAPULTA:
        nombre = "Catapulta";
        danioBase = 25.0f;
        cooldownBase = 3000;
        costo = 100;
        rango = 250.0f;
        color = QColor(101, 67, 33);
        break;
    case Arma::MAGICA:
        nombre = "Torre Mágica";
        danioBase = 12.0f;
        cooldownBase = 1500;
        costo = 80;
        rango = 200.0f;
        color = QColor(75, 0, 130);
        break;
    default:
        nombre = "Torre Genérica";
        danioBase = 10.0f;
        cooldownBase = 1500;
        costo = 50;
        rango = 200.0f;
        color = QColor(128, 128, 128);
        break;
    }

    danio = danioBase;
    cooldown = cooldownBase;
    tiempoCooldownRestante = 0;

    // Inicializar referencias
    posicionJugador = nullptr;
    direccionJugador = nullptr;
}

Torre::~Torre()
{
    // El destructor base de Arma se encarga de la limpieza
}

void Torre::actualizar(float deltaTime)
{
    if(tiempoCooldownRestante > 0) {
        tiempoCooldownRestante -= deltaTime;
        if(tiempoCooldownRestante < 0) tiempoCooldownRestante = 0;
    }

    // Actualizar proyectiles existentes usando el método de Arma
    actualizarProyectiles(deltaTime);
    actualizarSpritesAtaque(deltaTime);
}

bool Torre::puedeAtacar() const
{
    return tiempoCooldownRestante <= 0 && !enemigosCercanos.isEmpty();
}

void Torre::activar(const QPointF& posicion, const QPointF& direccion)
{
    Q_UNUSED(posicion); // Usamos this->posicion en lugar del parámetro
    Q_UNUSED(direccion);

    if(!puedeAtacar()) return;

    // Buscar el enemigo más cercano dentro del rango
    Enemigo* objetivo = nullptr;
    float distanciaMasCercana = std::numeric_limits<float>::max();

    for(Enemigo* enemigo : enemigosCercanos) {
        if(enemigo && enemigo->estaViva()) {
            QPointF distancia = enemigo->getPosicion() - this->posicion;
            float magnitud = qSqrt(distancia.x() * distancia.x() + distancia.y() * distancia.y());

            if(magnitud <= rango && magnitud < distanciaMasCercana) {
                distanciaMasCercana = magnitud;
                objetivo = enemigo;
            }
        }
    }

    if(objetivo) {
        QPointF direccionAtaque = objetivo->getPosicion() - this->posicion;
        float magnitud = qSqrt(direccionAtaque.x() * direccionAtaque.x() + direccionAtaque.y() * direccionAtaque.y());

        if(magnitud > 0) {
            direccionAtaque /= magnitud;
        }

        crearProyectil(direccionAtaque);
        tiempoCooldownRestante = cooldown;
    }
}

void Torre::crearProyectil(const QPointF& direccion)
{
    // Usar los contenedores heredados de Arma
    proyectiles.append(posicion);
    direccionesProyectiles.append(direccion);
    tiemposVidaProyectiles.append(2000.0f);

    // Crear sprite del proyectil
    ProyectilSprite proyectilSprite;
    proyectilSprite.posicion = posicion;
    proyectilSprite.direccion = direccion;
    proyectilSprite.tiempoVida = 2000.0f;
    proyectilSprite.rotacion = std::atan2(direccion.y(), direccion.x()) * 180 / M_PI;
    proyectilSprite.frameActual = 0;
    proyectilesSprites.append(proyectilSprite);
}

void Torre::mejorar()
{
    nivel++;
    danio = danioBase * (1.0f + (nivel - 1) * 0.2f);
    cooldown = cooldownBase * (1.0f - (nivel - 1) * 0.1f);
    rango += 20.0f;

    qDebug() << "¡Torre" << nombre << "mejorada a nivel" << nivel << "!";
}
