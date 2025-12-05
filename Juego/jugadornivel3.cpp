#include "jugadornivel3.h"
#include <QtMath>
#include <QDebug>

JugadorNivel3::JugadorNivel3()
{
    // Configuración específica para el nivel 3
    vida = 5;
    velocidad = 3.0f;
    posicion = QPointF(100, 530); // MÁS ABAJO - en el suelo

    // Estados del nivel 3
    estaSaltando = false;
    estaAgachado = false;
    tiempoSalto = 0.0f;
    velocidadVertical = 0.0f;
    gravedadAumentada = false;

    // Física
    alturaSalto = 280.0f;
    duracionSalto = 0.7f;
    gravedadNormal = 1000.0f;   // Gravedad normal
    gravedadRapida = 2000.0f;   // Gravedad rápida para caída
    gravedad = gravedadNormal;  // Comienza con gravedad normal

    qDebug() << "🎮 JugadorNivel3 creado - Gravedad rápida:" << gravedadRapida;
}

JugadorNivel3::~JugadorNivel3()
{
    // Limpiar armas si las hubiera
    // USAR for normal en lugar de range-for para evitar warning
    for (int i = 0; i < armas.size(); ++i) {
        delete armas[i];
    }
    armas.clear();
}

void JugadorNivel3::actualizar(float deltaTime)
{
    float deltaSec = deltaTime / 1000.0f;

    // DEBUG: Mostrar estado ocasionalmente
    static int debugCounter = 0;
    if (debugCounter++ % 60 == 0) {
        qDebug() << "🔄 Jugador actualizando - Saltando:" << estaSaltando
                 << "GravedadAumentada:" << gravedadAumentada
                 << "VelVertical:" << velocidadVertical
                 << "PosY:" << posicion.y();
    }

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
        gravedadAumentada = false;
        gravedad = gravedadNormal; // Restaurar gravedad normal

        if (debugCounter % 60 == 0) {
            qDebug() << "👣 Jugador tocó suelo - Gravedad restaurada a normal";
        }
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
        velocidadVertical = -sqrt(2.0f * gravedadNormal * alturaSalto);
        gravedad = gravedadNormal; // Asegurar gravedad normal al saltar
        gravedadAumentada = false;

        qDebug() << "🦘 Salto iniciado - VelVertical:" << velocidadVertical;
    }
}

void JugadorNivel3::agacharse()
{
    if (!estaSaltando && estaEnSuelo()) {
        estaAgachado = true;
        qDebug() << "👇 Agachado en suelo";
    } else if (estaSaltando) {
        // Si está saltando, aumentar gravedad para caer más rápido
        aumentarGravedad();
        qDebug() << "🚀 Caída rápida activada durante salto";
    }
}

void JugadorNivel3::levantarse()
{
    if (estaAgachado) {
        estaAgachado = false;
        if (!estaSaltando) {
            // Solo restaurar gravedad si no está saltando
            gravedadAumentada = false;
            gravedad = gravedadNormal;
        }
        qDebug() << "🔼 Levantado";
    }
}

void JugadorNivel3::cancelarSalto()
{
    if (estaSaltando) {
        // Aumentar gravedad para caída rápida
        gravedadAumentada = true;
        gravedad = gravedadRapida;

        // Si está muy arriba, dar un impulso hacia abajo
        if (posicion.y() < 400.0f) { // Si está por encima de 400px
            velocidadVertical = qMax(velocidadVertical, 300.0f); // Impulso hacia abajo mínimo
            qDebug() << "💥 Cancelar salto - Impulso hacia abajo";
        } else {
            qDebug() << "💥 Cancelar salto - Gravedad aumentada";
        }
    }
}

void JugadorNivel3::aumentarGravedad()
{
    if (estaSaltando && !gravedadAumentada) {
        gravedadAumentada = true;
        gravedad = gravedadRapida;
        qDebug() << "⬇️ Gravedad aumentada a:" << gravedad;
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
        return QRectF(posicion.x() - 20, posicion.y() - 8, 40, 16); // Más bajo y ancho
    } else {
        return QRectF(posicion.x() - 15, posicion.y() - 20, 30, 35); // Más bajo
    }
}

bool JugadorNivel3::estaEnSuelo() const {
    return posicion.y() >= 530.0f - 0.1f; // Ajustado a la nueva posición
}
