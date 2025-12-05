#ifndef JUGADORNIVEL2_H
#define JUGADORNIVEL2_H

#include <QPointF>
#include <QRectF>
#include <vector>

class JugadorNivel2
{
public:
    JugadorNivel2();
    ~JugadorNivel2() = default;

    void actualizar(float deltaTime);
    void procesarInput(const std::vector<bool>& teclas);

    void moverDerecha();
    void moverIzquierda();

    QPointF getPosicion() const { return posicion; }
    float getVida() const { return vida; }
    void recibirDanio(float danio) { vida -= danio; }

    QRectF getAreaColision() const;

private:
    float vida = 100.0f;
    QPointF posicion;
    std::vector<bool> teclasPresionadas;

    float velocidadMovimientoHorizontal = 8.0f;
    int limiteIzquierdo = 50;
    int limiteDerecho = 974;
};

#endif // JUGADORNIVEL2_H
