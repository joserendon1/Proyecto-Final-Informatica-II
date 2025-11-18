#ifndef JUGADORNIVEL1_H
#define JUGADORNIVEL1_H

#include "entidad.h"
#include "arma.h"
#include <QPointF>
#include <QList>

class JugadorNivel1 : public Entidad
{
public:
    JugadorNivel1();
    ~JugadorNivel1();

    void actualizar(float deltaTime) override;
    QRectF getAreaColision() const override;

    void procesarInput(bool teclas[]);
    void activarArmas();

    // Sistema de experiencia
    void ganarExperiencia(int exp);
    void subirNivel();
    bool tieneMejoraPendiente() const { return mejoraPendiente; }
    void setMejoraPendiente(bool pendiente) { mejoraPendiente = pendiente; }

    // Métodos para aplicar mejoras
    void aplicarMejoraVida(float extra);
    void aplicarMejoraDanio(float extra);
    void aplicarMejoraVelocidad(float extra);
    void anadirArmaNueva(Arma::Tipo tipoArma);
    void mejorarArmaExistente(Arma::Tipo tipoArma);

    // Getters
    int getNivel() const { return nivel; }
    int getExperiencia() const { return experiencia; }
    int getExperienciaParaSiguienteNivel() const;
    float getDanioExtra() const { return danioExtra; }
    float getVelocidadExtra() const { return velocidadExtra; }
    QList<Arma*> getArmas() const { return armas; }
    bool tieneArma(Arma::Tipo tipo) const; // NUEVO método

private:
    bool teclasPresionadas[4]; // WASD
    QPointF velocidadMovimiento;
    QPointF ultimaDireccion;
    QList<Arma*> armas;
    int nivel;
    int experiencia;
    bool mejoraPendiente;
    float danioExtra;
    float velocidadExtra;
};

#endif // JUGADORNIVEL1_H
