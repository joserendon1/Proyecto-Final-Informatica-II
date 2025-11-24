#ifndef NIVEL3_H
#define NIVEL3_H

#include "nivelbase.h"
#include "jugadornivel3.h"
#include <QTimer>
#include <QElapsedTimer>

class Nivel3 : public NivelBase
{
    Q_OBJECT

public:
    explicit Nivel3(QWidget *parent = nullptr);
    ~Nivel3();

    void iniciarNivel() override;
    void pausarNivel() override;
    void reanudarNivel() override;

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void actualizarJuego(float deltaTime) override;

private:
    void setupNivel();
    void generarObstaculos();
    void verificarColisiones();
    void actualizarCamaraAutoScroll();
    void dibujarHUD(QPainter &painter);

    // Propiedades específicas del nivel 3
    float velocidadScroll;
    float distanciaRecorrida;
    QElapsedTimer timerNivel;
    QList<QRectF> obstaculos;
    QList<QRectF> powerUps;

    // Configuración
    float tiempoObjetivo;
    float spawnRate;
    float tiempoDesdeUltimoSpawn;

    // Jugador específico para nivel 3
    JugadorNivel3* jugadorN3;

    // Estados del juego
    bool nivelCompletado;
    bool juegoActivo;
};

#endif // NIVEL3_H
