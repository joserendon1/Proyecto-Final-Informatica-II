#ifndef TORRE_H
#define TORRE_H

#include "arma.h"
#include <QList>

class Torre : public Arma
{
public:
    Torre(Arma::Tipo tipo);
    virtual ~Torre();

    void actualizar(float deltaTime) override; // override correcto ahora
    bool puedeAtacar() const;
    void activar(const QPointF& posicion, const QPointF& direccion = QPointF(0, -1)) override; // override correcto ahora

    // Nuevos métodos específicos de torres
    void mejorar();
    int getCosto() const { return costo; }
    int getCostoMejora() const { return costo * nivel; }
    int getCostoVenta() const { return (costo * nivel) / 2; }
    float getRango() const { return rango; }
    int getNivel() const { return nivel; }
    QPointF getPosicion() const { return posicion; }
    void setPosicion(const QPointF& nuevaPos) { posicion = nuevaPos; }

private:
    void crearProyectil(const QPointF& direccion);

    QPointF posicion;
    float rango = 200.0f;
    int nivel = 1;
    int costo = 50;
};

#endif // TORRE_H
