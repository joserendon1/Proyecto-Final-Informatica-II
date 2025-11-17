#ifndef MEJORA_H
#define MEJORA_H

#include <QString>

class JugadorNivel1; // Forward declaration

class Mejora
{
public:
    enum Tipo {
        VIDA,
        DANIO,
        VELOCIDAD,
        ARMA  // NUEVO tipo de mejora
    };

    Mejora(Tipo tipo, const QString& nombre, const QString& descripcion, float valor = 0, int tipoArma = -1);

    void aplicar(JugadorNivel1* jugador) const;

    Tipo getTipo() const { return tipo; }
    QString getNombre() const { return nombre; }
    QString getDescripcion() const { return descripcion; }
    float getValor() const { return valor; }
    int getTipoArma() const { return tipoArma; } // NUEVO getter para tipo de arma

private:
    Tipo tipo;
    QString nombre;
    QString descripcion;
    float valor;
    int tipoArma; // NUEVO: para identificar qué arma agregar
};

#endif // MEJORA_H
