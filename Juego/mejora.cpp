#include "mejora.h"
#include "jugadornivel1.h"
#include "arma.h"

Mejora::Mejora(Tipo tipo, const QString& nombre, const QString& descripcion, float valor, int tipoArma)
    : tipo(tipo), nombre(nombre), descripcion(descripcion), valor(valor), tipoArma(tipoArma)
{
}

void Mejora::aplicar(JugadorNivel1* jugador) const
{
    if (!jugador) return;

    switch(tipo) {
    case VIDA:
        jugador->aplicarMejoraVida(valor);
        break;
    case DANIO:
        jugador->aplicarMejoraDanio(valor);
        break;
    case VELOCIDAD:
        jugador->aplicarMejoraVelocidad(valor);
        break;
    case ARMA:
        jugador->anadirArmaNueva(static_cast<Arma::Tipo>(tipoArma));
        break;
    }
}
