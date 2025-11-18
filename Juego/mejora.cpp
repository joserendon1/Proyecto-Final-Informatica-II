#include "mejora.h"
#include "jugadornivel1.h"
#include "arma.h"

Mejora::Mejora(Tipo tipo, const QString& nombre, const QString& descripcion, int tipoArma)
    : tipo(tipo), nombre(nombre), descripcion(descripcion), tipoArma(tipoArma)
{
}

void Mejora::aplicar(JugadorNivel1* jugador) const
{
    if (!jugador) return;

    if(tipo == ARMA) {
        jugador->anadirArmaNueva(static_cast<Arma::Tipo>(tipoArma));
    }
}
