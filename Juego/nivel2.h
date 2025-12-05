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
    struct Barril {
        QPointF posicion;
        float velocidad;
        bool activo;
        int tipo; // 1: normal (obstacle3), 2: grande (obstacle1), 3: pequeño (obstacle4)

        Barril(QPointF pos, float vel, int t = 1) :
            posicion(pos), velocidad(vel), activo(true), tipo(t) {}

        QRectF getAreaColision() const {
            switch(tipo) {
            case 1: return QRectF(posicion.x() - 15, posicion.y() - 15, 30, 30); // obstacle3
            case 2: return QRectF(posicion.x() - 25, posicion.y() - 25, 50, 50); // obstacle1
            case 3: return QRectF(posicion.x() - 10, posicion.y() - 10, 20, 20); // obstacle4
            default: return QRectF(posicion.x() - 15, posicion.y() - 15, 30, 30);
            }
        }

        QString getSpriteName() const {
            switch(tipo) {
            case 1: return "obstacle3";
            case 2: return "obstacle1";
            case 3: return "obstacle4";
            default: return "obstacle3";
            }
        }

        QSize getDisplaySize() const {
            switch(tipo) {
            case 1: return QSize(30, 30); // obstacle3
            case 2: return QSize(50, 50); // obstacle1
            case 3: return QSize(20, 20); // obstacle4
            default: return QSize(30, 30);
            }
        }

        bool estaFueraDePantalla() const {
            return posicion.y() > 768;
        }
    };

    void setupNivel();
    void procesarColisiones();
    void limpiarBarriles();
    void generarBarril();
    void actualizarAnimacion(float deltaTime);
    void dibujarJugador(QPainter &painter);
    void dibujarBarriles(QPainter &painter);
    void dibujarHUD(QPainter &painter);
    void dibujarFondo(QPainter &painter);
    void dibujarSuelo(QPainter &painter);

    // Métodos helper similares al Nivel 1
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
    const int ANCHO_VENTANA = 1024;
    const int ALTO_VENTANA = 768;
    const int SUELO_Y = 700;
    const int LIMITE_IZQUIERDO = 50;
    const int LIMITE_DERECHO = 974;
};

#endif // NIVEL2_H
