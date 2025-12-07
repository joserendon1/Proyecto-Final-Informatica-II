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
    const int SUELO_Y = 550;
    const int AJUSTE_OBSTACULO1 = 35;
    const int AJUSTE_OBSTACULO3 = 35;
    const int SEPARACION_MINIMA = 600;
    const int SEPARACION_MAXIMA = 800;
    void setupNivel();
    void generarObstaculos();
    void generarPatronObstaculos(int tipoPatron);
    void generarObstaculosAleatorios();
    void verificarColisiones();
    void actualizarCamaraAutoScroll();
    void actualizarAnimacion(float deltaTime);
    void dibujarHUD(QPainter &painter);
    void dibujarJugador(QPainter &painter);
    void dibujarSueloConSprite(QPainter &painter);
    void dibujarObstaculosConSprites(QPainter &painter);
    QSize obtenerDimensionesSprite(int tipoObstaculo);
    QSize obtenerDimensionesHitbox(int tipoObstaculo);
    int obtenerAjusteY(int tipoObstaculo);
    float velocidadScroll;
    float distanciaRecorrida;
    QElapsedTimer timerNivel;
    QList<QRectF> obstaculos;
    QList<QRectF> spriteRects;
    QList<int> tiposObstaculos;
    QList<QRectF> powerUps;
    float tiempoObjetivo;
    float spawnRate;
    float tiempoDesdeUltimoSpawn;
    int frameAnimacion;
    float tiempoAnimacion;
    JugadorNivel3* jugadorN3;
    bool nivelCompletado;
    bool juegoActivo;
    QList<QPair<float, int>> milestonesObstaculos;
    QList<int> getObstaculosDisponibles();
};

#endif // NIVEL3_H
