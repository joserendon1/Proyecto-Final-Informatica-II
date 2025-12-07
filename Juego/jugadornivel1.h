#ifndef JUGADORNIVEL1_H
#define JUGADORNIVEL1_H

#include "jugadorbase.h"
#include <QList>

class JugadorNivel1 : public JugadorBase
{
public:
    JugadorNivel1();
    virtual ~JugadorNivel1();
    void actualizar(float deltaTime) override;
    void procesarInput(const std::vector<bool>& teclas) override;
    void activarArmas() override;
    const QList<Arma*>& getArmas() const override { return armas; }
    bool tieneArma(Arma::Tipo tipo) const override;
    void anadirArmaNueva(Arma::Tipo tipoArma) override;
    void aplicarMejoraVida(float extra);
    void aplicarMejoraDanio(float extra);
    void aplicarMejoraVelocidad(float extra);
    QRectF getAreaColision() const override;
    void ganarExperiencia(int exp) override;
    void subirNivel() override;
    int getExperienciaParaSiguienteNivel() const override;

private:
    QList<Arma*> armas;
    float danioExtra = 0;
    float velocidadExtra = 0;
};

#endif // JUGADORNIVEL1_H
