#ifndef NIVEL2_H
#define NIVEL2_H

#include <QWidget>
#include <QTimer>
#include <QList>
#include <QElapsedTimer>
#include "jugadornivel2.h"
#include "audiomanager.h"
#include "spritemanager.h"
#include "uimanager.h"

class Nivel2 : public QWidget
{
    Q_OBJECT

public:
    explicit Nivel2(QWidget *parent = nullptr);
    ~Nivel2();

    void iniciarNivel();
    void pausarNivel();
    void reanudarNivel();
    QSize getGameSize() const { return size(); }

signals:
    void gamePaused();
    void gameResumed();
    void gameOver();
    void levelCompleted();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void actualizarJuego();

private:
    // Declaración forward de la estructura Barril
    struct Barril;

    void setupNivel();
    void procesarColisiones();
    void limpiarBarriles();
    void generarBarril();
    void actualizarAnimacion(float deltaTime);
    void dibujarJugador(QPainter &painter);
    void dibujarBarriles(QPainter &painter);
    void dibujarHUD(QPainter &painter);
    void dibujarFondo(QPainter &painter);
    void dibujarFondoEnCapas(QPainter &painter);
    void dibujarSuelo(QPainter &painter);

    // Métodos helper para dibujo
    void dibujarEntidadConSprite(QPainter &painter, const QPointF &posicion,
                                 const QString &spriteName, const QSize &displaySize,
                                 int frameWidth, int frameHeight, int currentFrame);
    void dibujarEntidadSimple(QPainter &painter, const QPointF &posicion,
                              const QSize &displaySize, const QColor &color);

    QTimer* timerJuego;
    QTimer* timerGeneracionBarriles;
    QElapsedTimer timerNivel;

    JugadorNivel2* jugador;
    QList<Barril> barriles;

    // Animación
    int frameAnimacion;
    float tiempoAnimacion;

    // Estadísticas
    int tiempoTranscurrido;
    int tiempoObjetivo;
    int barrilesEsquivados;
    int puntuacion;

    // Estados
    bool enPausa;
    bool juegoIniciado;
    bool nivelCompletado;

    // Configuración
    float spawnRate;
    float tiempoDesdeUltimoSpawn;

    // Audio
    int cooldownSonidoMovimiento;
    bool jugadorSeEstaMoviendo;

    // Constantes
    const int ANCHO_VENTANA = 800;
    const int ALTO_VENTANA = 600;
    const int SUELO_Y = 550;
    const int LIMITE_IZQUIERDO = 50;
    const int LIMITE_DERECHO = 750;
};

#endif // NIVEL2_H
