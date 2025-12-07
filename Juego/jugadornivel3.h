#ifndef JUGADORNIVEL3_H
#define JUGADORNIVEL3_H

#include "jugadorbase.h"
#include "arma.h"
#include <QList>

class JugadorNivel3 : public JugadorBase
{
public:
    JugadorNivel3();
    ~JugadorNivel3();

    void actualizar(float deltaTime) override;
    void procesarInput(const std::vector<bool>& teclas) override;
    void activarArmas() override;

    const QList<Arma*>& getArmas() const override { return armas; }
    bool tieneArma(Arma::Tipo tipo) const override { Q_UNUSED(tipo); return false; }
    void anadirArmaNueva(Arma::Tipo tipoArma) override { Q_UNUSED(tipoArma); }

    void saltar();
    void agacharse();
    void levantarse();
    void cancelarSalto();
    void aumentarGravedad();
    void moverVertical(float direccion);
    QRectF getAreaColision() const override;
    bool estaSaltando;
    bool estaAgachado;
    bool estaEnSuelo() const;
    float tiempoSalto;
    float velocidadVertical;
    bool gravedadAumentada;
    float getVida() const { return vida; }
    void setVida(float nuevaVida) { vida = nuevaVida; }
    QPointF getPosicion() const { return posicion; }
    void setPosicionX(float x) { posicion.setX(x); }
    void setPosicionY(float y) { posicion.setY(y); }

private:
    float alturaSalto;
    float duracionSalto;
    float gravedadNormal;
    float gravedadRapida;
    float gravedad;
    QList<Arma*> armas;
};

#endif // JUGADORNIVEL3_H
