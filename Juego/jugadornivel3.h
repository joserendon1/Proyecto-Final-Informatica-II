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

    // Implementar funciones virtuales puras de JugadorBase
    void actualizar(float deltaTime) override;
    void procesarInput(const std::vector<bool>& teclas) override;
    void activarArmas() override;

    // Implementar funciones virtuales puras para armas (vacías para nivel 3)
    const QList<Arma*>& getArmas() const override { return armas; }
    bool tieneArma(Arma::Tipo tipo) const override { Q_UNUSED(tipo); return false; }
    void anadirArmaNueva(Arma::Tipo tipoArma) override { Q_UNUSED(tipoArma); }

    // Funciones específicas del nivel 3
    void saltar();
    void agacharse();
    void levantarse();
    void cancelarSalto();      // Nueva función
    void aumentarGravedad();   // Nueva función
    void moverVertical(float direccion);  // AÑADIR ESTA LÍNEA

    // Override de funciones base
    QRectF getAreaColision() const override;

    // Estados específicos del nivel 3
    bool estaSaltando;
    bool estaAgachado;
    bool estaEnSuelo() const;
    float tiempoSalto;
    float velocidadVertical;
    bool gravedadAumentada;    // Nuevo estado

    // Getters para acceder a miembros protegidos
    float getVida() const { return vida; }
    void setVida(float nuevaVida) { vida = nuevaVida; }
    QPointF getPosicion() const { return posicion; }
    void setPosicionX(float x) { posicion.setX(x); }
    void setPosicionY(float y) { posicion.setY(y); }

private:
    // Configuración específica del nivel 3
    float alturaSalto;
    float duracionSalto;
    float gravedadNormal;      // Gravedad normal
    float gravedadRapida;      // Gravedad rápida para caída
    float gravedad;            // Gravedad actual

    // Lista de armas vacía (no se usan en nivel 3)
    QList<Arma*> armas;
};

#endif // JUGADORNIVEL3_H
