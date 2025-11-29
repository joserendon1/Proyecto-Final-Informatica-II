#ifndef JUGADORNIVEL2_H
#define JUGADORNIVEL2_H

#include "jugadorbase.h"
#include <QList>

class JugadorNivel2 : public JugadorBase
{
public:
    JugadorNivel2();
    virtual ~JugadorNivel2();

    // Implementación de métodos virtuales puros
    void actualizar(float deltaTime) override;
    void procesarInput(const std::vector<bool>& teclas) override;
    void activarArmas() override;

    // Implementación de métodos virtuales de armas
    const QList<Arma*>& getArmas() const override;
    bool tieneArma(Arma::Tipo tipo) const override;
    void anadirArmaNueva(Arma::Tipo tipoArma) override; // Asegúrate de que esté declarado

    // Métodos específicos del nuevo nivel 2
    void moverDerecha();
    void moverIzquierda();
    QRectF getAreaColision() const override;

    // Getters
    float getVelocidadMovimiento() const { return velocidadMovimientoHorizontal; }

private:
    float velocidadMovimientoHorizontal = 5.0f;
    int limiteIzquierdo = 50;
    int limiteDerecho = 974; // 1024 - 50
};

#endif // JUGADORNIVEL2_H
