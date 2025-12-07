#ifndef JUGADORNIVEL2_H
#define JUGADORNIVEL2_H

#include "jugadorbase.h"
#include <QPointF>
#include <QRectF>
#include <vector>
#include <QList>

class Arma;

class JugadorNivel2 : public JugadorBase
{
public:
    JugadorNivel2();
    ~JugadorNivel2() override = default;
    void actualizar(float deltaTime) override;
    void procesarInput(const std::vector<bool>& teclas) override;
    void activarArmas() override;
    const QList<Arma*>& getArmas() const override;
    bool tieneArma(Arma::Tipo tipo) const override;
    void anadirArmaNueva(Arma::Tipo tipoArma) override;
    void moverDerecha();
    void moverIzquierda();
    void resetear();
    QRectF getAreaColision() const override;
    bool estaMoviendose() const { return seEstaMoviendo; }
    int getDireccion() const { return direccionActual; }
    void recibirDanio(float danio) { vida -= danio; if (vida < 0) vida = 0; }

private:
    bool seEstaMoviendo = false;
    int direccionActual = 0;
    int limiteIzquierdo = 50;
    int limiteDerecho = 750;
    QList<Arma*> armas;
};

#endif // JUGADORNIVEL2_H
