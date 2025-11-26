#include "jugadornivel3.h"
#include <QtMath>
#include <QDebug>

JugadorNivel3::JugadorNivel3()
{
    // Configuración específica para el nivel 3
    vida = 5;
    velocidad = 3.0f;
    posicion = QPointF(100, 530); // MÁS ABAJO - en el suelo (antes 500)

    // Estados del nivel 3
    estaSaltando = false;
    estaAgachado = false;
    tiempoSalto = 0.0f;
    velocidadVertical = 0.0f;

    // Física
    alturaSalto = 280.0f;
    duracionSalto = 0.7f;
    gravedad = 1000.0f;
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
    float deltaSec = deltaTime / 1000.0f;

    // Aplicar gravedad si no está en el suelo
    if (!estaEnSuelo()) {
        velocidadVertical += gravedad * deltaSec;
    }

    // Actualizar posición vertical
    posicion.setY(posicion.y() + velocidadVertical * deltaSec);

    // Verificar si llegó al suelo (530 es el suelo)
    if (posicion.y() >= 530.0f) {
        posicion.setY(530.0f);
        velocidadVertical = 0.0f;
        estaSaltando = false;
        tiempoSalto = 0.0f;
    }

    // Limitar posición vertical mínima
    if (posicion.y() < 100.0f) {
        posicion.setY(100.0f);
        velocidadVertical = 0.0f;
    }
}

void JugadorNivel3::procesarInput(const std::vector<bool>& teclas)
{
    Q_UNUSED(teclas);
    // Input manejado directamente en key events del nivel 3
}

void JugadorNivel3::activarArmas()
{
    // No hay armas en nivel 3
}

void JugadorNivel3::saltar()
{
    if (!estaSaltando && !estaAgachado && estaEnSuelo()) {
        estaSaltando = true;
        tiempoSalto = 0.0f;
        velocidadVertical = -sqrt(2.0f * gravedad * alturaSalto);
    }
}

void JugadorNivel3::agacharse()
{
    if (!estaSaltando && estaEnSuelo()) {
        estaAgachado = true;
    }
}

void JugadorNivel3::levantarse()
{
    estaAgachado = false;
}

void JugadorNivel3::moverVertical(float direccion)
{
    if (!estaSaltando && !estaAgachado) {
        velocidadVertical = direccion * velocidad * 50.0f;
    }
}

QRectF JugadorNivel3::getAreaColision() const
{
    if (estaAgachado) {
        return QRectF(posicion.x() - 20, posicion.y() - 8, 40, 16); // Más bajo y ancho
    } else {
        return QRectF(posicion.x() - 15, posicion.y() - 20, 30, 35); // Más bajo
    }
}

bool JugadorNivel3::estaEnSuelo() const {
    return posicion.y() >= 530.0f - 0.1f; // Ajustado a la nueva posición
}
