#ifndef JUGADORBASE_H
#define JUGADORBASE_H

#include "entidad.h"
#include "arma.h"
#include <QPointF>
#include <vector>

class JugadorBase : public Entidad
{
public:
    JugadorBase();
    virtual ~JugadorBase() = default;

    // Métodos virtuales puros que deben implementar las clases derivadas
    virtual void actualizar(float deltaTime) = 0;
    virtual void procesarInput(const std::vector<bool>& teclas) = 0;
    virtual void activarArmas() = 0;

    // Métodos virtuales con implementación por defecto
    virtual QRectF getAreaColision() const;
    virtual void ganarExperiencia(int exp);
    virtual void subirNivel();
    virtual int getExperienciaParaSiguienteNivel() const;

    // Getters y setters comunes
    float getVida() const { return vida; }
    QPointF getPosicion() const { return posicion; }
    void setPosicion(const QPointF& nuevaPos) { posicion = nuevaPos; }
    float getVelocidad() const { return velocidad; }
    void setVelocidad(float nuevaVel) { velocidad = nuevaVel; }
    int getNivel() const { return nivel; }
    int getExperiencia() const { return experiencia; }
    bool estaViva() const { return vida > 0; }
    bool tieneMejoraPendiente() const { return mejoraPendiente; }
    void setMejoraPendiente(bool pendiente) { mejoraPendiente = pendiente; }

    // Métodos virtuales para armas (implementados en clases específicas)
    virtual const QList<Arma*>& getArmas() const = 0;
    virtual bool tieneArma(Arma::Tipo tipo) const = 0;
    virtual void anadirArmaNueva(Arma::Tipo tipoArma) = 0;

protected:
    int nivel = 1;
    int experiencia = 0;
    bool mejoraPendiente = false;
    std::vector<bool> teclasPresionadas;
    QPointF ultimaDireccion;
    QPointF velocidadMovimiento;
};

#endif // JUGADORBASE_H
