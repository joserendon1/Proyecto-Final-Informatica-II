#ifndef JUGADORNIVEL2_H
#define JUGADORNIVEL2_H

#include "jugadorbase.h"
#include "torre.h"
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
    void anadirArmaNueva(Arma::Tipo tipoArma) override;

    // Métodos específicos de JugadorNivel2
    bool construirTorre(Torre::Tipo tipo, const QPointF& posicion);
    void mejorarTorre(Torre* torre);
    void venderTorre(Torre* torre);
    Torre* getTorreEnPosicion(const QPointF& posicion) const;

    // Getters para recursos
    int getRecursos() const { return recursos; }
    void agregarRecursos(int cantidad) { recursos += cantidad; }
    bool gastarRecursos(int cantidad);

    // Override de métodos de JugadorBase
    QRectF getAreaColision() const override;

    // Getter específico para torres
    const QList<Torre*>& getTorres() const { return torres; }

private:
    QList<Torre*> torres;
    mutable QList<Arma*> armasCache; // Cache para compatibilidad
    int recursos = 100;
    QPointF posicionBase;
};

#endif // JUGADORNIVEL2_H
