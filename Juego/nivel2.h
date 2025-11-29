#ifndef NIVEL2_H
#define NIVEL2_H

#include "nivelbase.h"
#include "jugadornivel2.h"
#include <QTimer>
#include <QList>

class Nivel2 : public NivelBase
{
    Q_OBJECT

public:
    explicit Nivel2(QWidget *parent = nullptr);
    ~Nivel2();

    void actualizarJuego(float deltaTime) override;
    void iniciarNivel() override;
    void pausarNivel() override;
    void reanudarNivel() override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void generarObstaculo();

private:
    // Estructura simple para obstáculos
    struct Obstaculo {
        QPointF posicion;
        int tipo; // 1: Normal, 2: Grande, 3: Rápido
        float velocidad;
        bool activo;

        Obstaculo(QPointF pos, int t, float vel)
            : posicion(pos), tipo(t), velocidad(vel), activo(true) {}

        QRectF getAreaColision() const {
            float tamano;
            switch(tipo) {
            case 1: tamano = 15.0f; break; // Normal
            case 2: tamano = 25.0f; break; // Grande
            case 3: tamano = 10.0f; break; // Rápido
            default: tamano = 15.0f;
            }
            return QRectF(posicion.x() - tamano, posicion.y() - tamano, tamano * 2, tamano * 2);
        }

        bool estaFueraDePantalla() const {
            return posicion.y() > 768;
        }
    };

    void procesarColisiones();
    void limpiarObstaculos();
    void dibujarJugador(QPainter &painter);
    void dibujarObstaculos(QPainter &painter);
    void dibujarHUD(QPainter &painter);

    QTimer* timerGeneracionObstaculos;
    QList<Obstaculo> obstaculos;

    int tiempoTranscurrido = 0;
    int tiempoObjetivo = 90;
    int obstaculosEsquivados = 0;
    float frecuenciaGeneracion = 1000.0f;
};

#endif // NIVEL2_H
