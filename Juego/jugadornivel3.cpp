#include "jugadornivel3.h"
#include <QtMath>
#include <QDebug>

JugadorNivel3::JugadorNivel3()
{
    // Configuración específica para el nivel 3
    vida = 3; // Vidas en lugar de salud
    velocidad = 3.0f;
    posicion = QPointF(100, 300); // Posición inicial fija

    // Estados del nivel 3
    estaSaltando = false;
    estaAgachado = false;
    tiempoSalto = 0.0f;
    velocidadVertical = 0.0f;

    // Configuración de física
    alturaSalto = 200.0f;
    duracionSalto = 0.8f;
    gravedad = 800.0f;

    // Inicializar lista de armas vacía
    armas = QList<Arma*>();
}

JugadorNivel3::~JugadorNivel3()
{
    // Limpiar armas si las hubiera
    for (Arma* arma : armas) {
        delete arma;
    }
    armas.clear();
}

void JugadorNivel3::actualizar(float deltaTime)
{
    // En el nivel 3, el movimiento se controla de forma diferente
    // El jugador solo se mueve verticalmente (saltar/agacharse)
    // El scroll horizontal es automático

    // Aplicar gravedad y movimiento vertical
    if (estaSaltando) {
        tiempoSalto += deltaTime / 1000.0f;

        // Física de salto más realista
        float progreso = tiempoSalto / duracionSalto;
        if (progreso <= 0.5f) {
            // Fase ascendente del salto
            velocidadVertical = -alturaSalto * (1.0f - progreso * 2.0f);
        } else {
            // Fase descendente del salto
            velocidadVertical = alturaSalto * ((progreso - 0.5f) * 2.0f);
        }

        if (tiempoSalto >= duracionSalto) {
            estaSaltando = false;
            tiempoSalto = 0.0f;
            velocidadVertical = 0.0f;
        }
    } else if (!estaAgachado) {
        // Aplicar gravedad suave cuando no está saltando
        velocidadVertical += gravedad * deltaTime / 1000.0f;
    }

    // Limitar velocidad vertical máxima
    velocidadVertical = qBound(-400.0f, velocidadVertical, 400.0f);

    // Aplicar movimiento vertical
    posicion.setY(posicion.y() + velocidadVertical * deltaTime / 1000.0f);

    // Limitar posición vertical (evitar que salga de la pantalla)
    posicion.setY(qMax(100.0f, qMin(500.0f, posicion.y())));

    // Si está en el suelo, resetear velocidad vertical
    if (posicion.y() >= 500.0f) {
        posicion.setY(500.0f);
        velocidadVertical = 0.0f;
    }
}

void JugadorNivel3::procesarInput(const std::vector<bool>& teclas)
{
    Q_UNUSED(teclas);
    // En el nivel 3, el input se maneja directamente en keyPressEvent
    // Esta función se mantiene vacía para cumplir con la interfaz
    qDebug() << "Input procesado en Nivel3 - teclas:" << teclas.size();
}

void JugadorNivel3::activarArmas()
{
    // En el nivel 3 no hay armas, esta función se mantiene vacía
    qDebug() << "Armas activadas en Nivel3 - pero no hay armas en este nivel";
}

void JugadorNivel3::saltar()
{
    if (!estaSaltando && !estaAgachado && posicion.y() >= 490.0f) {
        estaSaltando = true;
        tiempoSalto = 0.0f;
        velocidadVertical = -alturaSalto * 0.5f; // Impulso inicial
        qDebug() << "Jugador salta - posición Y:" << posicion.y();
    }
}

void JugadorNivel3::agacharse()
{
    if (!estaSaltando) {
        estaAgachado = true;
        qDebug() << "Jugador se agacha";
    }
}

void JugadorNivel3::levantarse()
{
    estaAgachado = false;
    qDebug() << "Jugador se levanta";
}

void JugadorNivel3::moverVertical(float direccion)
{
    // Movimiento vertical suave (podría usarse para plataformas móviles)
    if (!estaSaltando && !estaAgachado) {
        velocidadVertical = direccion * velocidad * 50.0f;
    }
}

QRectF JugadorNivel3::getAreaColision() const
{
    QRectF areaBase = QRectF(posicion.x() - 15, posicion.y() - 15, 30, 30);

    // Ajustar área de colisión según estado
    if (estaAgachado) {
        areaBase.setHeight(areaBase.height() * 0.6f);
        areaBase.moveTop(areaBase.top() + 12); // Bajar el centro visualmente
    }
    if (estaSaltando) {
        areaBase.moveTop(areaBase.top() - 20); // Ajustar posición en salto
    }

    return areaBase;
}
