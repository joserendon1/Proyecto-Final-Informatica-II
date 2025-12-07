#ifndef JUGADORNIVEL2_H
#define JUGADORNIVEL2_H

#include "jugadorbase.h"
#include <QPointF>
#include <QRectF>
#include <vector>
#include <QList>

// Forward declaration
class Arma;

class JugadorNivel2 : public JugadorBase
{
public:
    JugadorNivel2();
    ~JugadorNivel2() override = default;

    // Implementar métodos virtuales puros de JugadorBase
    void actualizar(float deltaTime) override;
    void procesarInput(const std::vector<bool>& teclas) override;
    void activarArmas() override;

    // Implementar métodos de armas
    const QList<Arma*>& getArmas() const override;
    bool tieneArma(Arma::Tipo tipo) const override;
    void anadirArmaNueva(Arma::Tipo tipoArma) override;

    // Métodos específicos de JugadorNivel2
    void moverDerecha();
    void moverIzquierda();
    void resetear();
    QRectF getAreaColision() const override;

    // Para animación
    bool estaMoviendose() const { return seEstaMoviendo; }
    int getDireccion() const { return direccionActual; }

    // Getters adicionales
    void recibirDanio(float danio) { vida -= danio; if (vida < 0) vida = 0; }

private:
    // Miembros específicos de JugadorNivel2
    bool seEstaMoviendo = false;
    int direccionActual = 0; // -1: izquierda, 1: derecha, 0: quieto

    // Límites de movimiento (específicos para Nivel2)
    int limiteIzquierdo = 50;
    int limiteDerecho = 750;

    // Lista de armas (requerido por las interfaces)
    QList<Arma*> armas;
};

#endif // JUGADORNIVEL2_H
