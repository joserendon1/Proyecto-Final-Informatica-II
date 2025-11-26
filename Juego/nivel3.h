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
    const int AJUSTE_OBSTACULO1 = 30;
    const int AJUSTE_OBSTACULO2 = 40;
    const int AJUSTE_OBSTACULO3 = 15;
    const int AJUSTE_OBSTACULO4 = 35;

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
    int determinarTipoObstaculo(const QRectF& obstaculo);

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

    // Animación
    int frameAnimacion;
    float tiempoAnimacion;

    // Jugador específico para nivel 3
    JugadorNivel3* jugadorN3;

    // Estados del juego
    bool nivelCompletado;
    bool juegoActivo;

    QList<QPair<float, int>> milestonesObstaculos;
    QList<int> getObstaculosDisponibles();
    QList<int> tiposObstaculos;
};

#endif // NIVEL3_H
