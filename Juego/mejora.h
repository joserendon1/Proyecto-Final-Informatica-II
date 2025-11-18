#ifndef MEJORA_H
#define MEJORA_H

#include <QString>

class JugadorNivel1;

class Mejora
{
public:
    enum Tipo {
        ARMA
    };

    Mejora(Tipo tipo, const QString& nombre, const QString& descripcion, int tipoArma);

    void aplicar(JugadorNivel1* jugador) const;

    Tipo getTipo() const { return tipo; }
    QString getNombre() const { return nombre; }
    QString getDescripcion() const { return descripcion; }
    int getTipoArma() const { return tipoArma; }

private:
    Tipo tipo;
    QString nombre;
    QString descripcion;
    int tipoArma;
};

#endif
