#include "jugadorbase.h"
#include <QtMath>

JugadorBase::JugadorBase()
{
    // Inicialización base común
    vida = 100.0f;
    velocidad = 2.0f;
    posicion = QPointF(400, 300);
    ultimaDireccion = QPointF(0, -1);

    // Tamaño base del vector de teclas
    teclasPresionadas.resize(4, false);
}

QRectF JugadorBase::getAreaColision() const {
    return QRectF(posicion.x() - 10, posicion.y() - 10, 20, 20);
}

void JugadorBase::ganarExperiencia(int exp) {
    experiencia += exp;
}

void JugadorBase::subirNivel() {
    nivel++;
    experiencia = 0;
    mejoraPendiente = true;
}

int JugadorBase::getExperienciaParaSiguienteNivel() const {
    return 100 * nivel;
}
