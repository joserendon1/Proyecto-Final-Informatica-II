#ifndef ENEMIGO_H
#define ENEMIGO_H

#include "entidad.h"
#include <QPointF>

class Enemigo : public Entidad
{
public:
    Enemigo(int tipoEnemigo = 1);

    void actualizar(float deltaTime) override;
    QRectF getAreaColision() const override;

    void seguirJugador(const QPointF& posicionJugador);
    void recibirDanio(float cantidad) override;

    // Getters
    int getTipo() const { return tipo; }
    int getExperienciaQueDa() const { return experienciaQueDa; }

private:
    int tipo; // 1: débil, 2: fuerte
    int experienciaQueDa;
};

#endif // ENEMIGO_H
