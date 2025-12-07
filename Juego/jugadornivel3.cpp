#include "jugadornivel3.h"
#include <QtMath>
#include <QDebug>

JugadorNivel3::JugadorNivel3()
{
    vida = 5;
    velocidad = 3.0f;
    posicion = QPointF(100, 540);

    estaSaltando = false;
    estaAgachado = false;
    tiempoSalto = 0.0f;
    velocidadVertical = 0.0f;
    gravedadAumentada = false;

    // Física
    alturaSalto = 280.0f;
    duracionSalto = 0.7f;
    gravedadNormal = 1000.0f;
    gravedadRapida = 2000.0f;
    gravedad = gravedadNormal;

    qDebug() << "JugadorNivel3 creado - Gravedad rápida:" << gravedadRapida;
}

JugadorNivel3::~JugadorNivel3()
{
    for (int i = 0; i < armas.size(); ++i) {
        delete armas[i];
    }
    armas.clear();
}

void JugadorNivel3::actualizar(float deltaTime)
{
    float deltaSec = deltaTime / 1000.0f;

    static int debugCounter = 0;
    if (debugCounter++ % 60 == 0) {
        qDebug() << "Jugador actualizando - Saltando:" << estaSaltando
                 << "GravedadAumentada:" << gravedadAumentada
                 << "VelVertical:" << velocidadVertical
                 << "PosY:" << posicion.y();
    }

    if (!estaEnSuelo()) {
        velocidadVertical += gravedad * deltaSec;
    }

    posicion.setY(posicion.y() + velocidadVertical * deltaSec);

    if (posicion.y() >= 540.0f) {
        posicion.setY(540.0f);
        velocidadVertical = 0.0f;
        estaSaltando = false;
        tiempoSalto = 0.0f;
        gravedadAumentada = false;
        gravedad = gravedadNormal;

        if (debugCounter % 60 == 0) {
            qDebug() << "Jugador tocó suelo - Gravedad restaurada a normal";
        }
    }

    if (posicion.y() < 100.0f) {
        posicion.setY(100.0f);
        velocidadVertical = 0.0f;
    }
}

void JugadorNivel3::procesarInput(const std::vector<bool>& teclas)
{
    Q_UNUSED(teclas);
}

void JugadorNivel3::activarArmas()
{
}

void JugadorNivel3::saltar()
{
    if (!estaSaltando && !estaAgachado && estaEnSuelo()) {
        estaSaltando = true;
        tiempoSalto = 0.0f;
        velocidadVertical = -sqrt(2.0f * gravedadNormal * alturaSalto);
        gravedad = gravedadNormal;
        gravedadAumentada = false;

        qDebug() << "Salto iniciado - VelVertical:" << velocidadVertical;
    }
}

void JugadorNivel3::agacharse()
{
    if (!estaSaltando && estaEnSuelo()) {
        estaAgachado = true;
        qDebug() << "Agachado en suelo";
    } else if (estaSaltando) {
        aumentarGravedad();
        qDebug() << "Caída rápida activada durante salto";
    }
}

void JugadorNivel3::levantarse()
{
    if (estaAgachado) {
        estaAgachado = false;
        if (!estaSaltando) {
            gravedadAumentada = false;
            gravedad = gravedadNormal;
        }
        qDebug() << "Levantado";
    }
}

void JugadorNivel3::cancelarSalto()
{
    if (estaSaltando) {
        gravedadAumentada = true;
        gravedad = gravedadRapida;

        if (posicion.y() < 400.0f) {
            velocidadVertical = qMax(velocidadVertical, 300.0f);
            qDebug() << "Cancelar salto - Impulso hacia abajo";
        } else {
            qDebug() << "Cancelar salto - Gravedad aumentada";
        }
    }
}

void JugadorNivel3::aumentarGravedad()
{
    if (estaSaltando && !gravedadAumentada) {
        gravedadAumentada = true;
        gravedad = gravedadRapida;
        qDebug() << "Gravedad aumentada a:" << gravedad;
    }
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
        return QRectF(posicion.x() - 20, posicion.y() - 8, 40, 16);
    } else {
        return QRectF(posicion.x() - 15, posicion.y() - 20, 30, 35);
    }
}

bool JugadorNivel3::estaEnSuelo() const {
    return posicion.y() >= 530.0f - 0.1f;
}
