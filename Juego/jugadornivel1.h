#ifndef JUGADORNIVEL1_H
#define JUGADORNIVEL1_H

#include "personaje.h"
#include "arma.h"
#include <QPointF>
#include <QList>
#include <Qdebug>

class JugadorNivel1 : public Personaje
{
public:
    JugadorNivel1();
    ~JugadorNivel1();

    void mover(const QPointF& direccion) override;
    void actualizar() override;

    void procesarInput(bool teclas[]);
    void recibirDanio(float cantidad);

    // Sistema de experiencia y nivel
    void ganarExperiencia(int exp);
    void subirNivel();

    // Sistema de armas
    void activarArmas();
    QList<Arma*> getArmas() const { return armas; }

    // Sistema de mejoras
    bool tieneMejoraPendiente() const { return mejoraPendiente; }
    void setMejoraPendiente(bool pendiente) { mejoraPendiente = pendiente; }

    // Métodos simples para aplicar mejoras
    void aplicarMejoraVida(float extra) {
        vida += extra;
    }

    void aplicarMejoraDanio(float extra) {
        danioExtra += extra;
        // Aplicar el daño extra a todas las armas
        for(Arma* arma : armas) {
            arma->setDanio(arma->getTipo() == Arma::ESPADA ? 25.0f + danioExtra : 15.0f + danioExtra);
        }
    }

    void aplicarMejoraVelocidad(float extra) {
        velocidadExtra += extra;
    }

    // Getters
    int getNivel() const { return nivel; }
    int getExperiencia() const { return experiencia; }
    int getExperienciaParaSiguienteNivel() const { return nivel * 100; }

    float getDanioExtra() const { return danioExtra; }
    float getVelocidadExtra() const { return velocidadExtra; }

private:
    bool teclasPresionadas[4]; // WASD
    int nivel;
    int experiencia;
    QPointF velocidadMovimiento;
    QList<Arma*> armas;

    // Sistema de mejoras
    bool mejoraPendiente;
    float danioExtra;
    float velocidadExtra;
};

#endif // JUGADORNIVEL1_H
