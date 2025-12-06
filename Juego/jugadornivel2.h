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

    void resetear();

    // Para animación
    bool estaMoviendose() const { return seEstaMoviendo; }
    int getDireccion() const { return direccionActual; } // -1: izquierda, 1: derecha, 0: quieto

    QPointF getPosicion() const { return posicion; }
    float getVida() const { return vida; }
    void recibirDanio(float danio) { vida -= danio; if (vida < 0) vida = 0; }

    QRectF getAreaColision() const;

private:
private:
    float vida = 100.0f;
    QPointF posicion;
    std::vector<bool> teclasPresionadas;

    // Para animación
    bool seEstaMoviendo = false;
    int direccionActual = 0; // -1: izquierda, 1: derecha, 0: quieto

    float velocidadMovimientoHorizontal = 3.0f;
    int limiteIzquierdo = 50;
    int limiteDerecho = 750;  // Ajustado de 974
};

#endif // JUGADORNIVEL2_H
