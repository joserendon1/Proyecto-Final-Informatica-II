#ifndef JUGADORNIVEL1_H
#define JUGADORNIVEL1_H

#include "jugadorbase.h"
#include <QList>

class JugadorNivel1 : public JugadorBase
{
public:
    JugadorNivel1();
    virtual ~JugadorNivel1();

    // Implementación de métodos virtuales puros
    void actualizar(float deltaTime) override;
    void procesarInput(const std::vector<bool>& teclas) override;
    void activarArmas() override;

    // Implementación de métodos virtuales de armas
    const QList<Arma*>& getArmas() const override { return armas; }
    bool tieneArma(Arma::Tipo tipo) const override;
    void anadirArmaNueva(Arma::Tipo tipoArma) override;

    // Métodos específicos de JugadorNivel1
    void aplicarMejoraVida(float extra);
    void aplicarMejoraDanio(float extra);
    void aplicarMejoraVelocidad(float extra);

    // Override de métodos de JugadorBase
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
