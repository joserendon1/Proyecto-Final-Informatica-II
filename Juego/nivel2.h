#ifndef NIVEL2_H
#define NIVEL2_H

#include <QWidget>
#include <QTimer>
#include <QList>
#include "jugadornivel2.h"

class Nivel2 : public QWidget
{
    Q_OBJECT

public:
    explicit Nivel2(QWidget *parent = nullptr);
    ~Nivel2();

    void iniciarNivel();
    void pausarNivel();
    void reanudarNivel();

signals:
    void gamePaused();
    void gameResumed();
    void gameOver();
    void levelCompleted();
    // Añadimos esta señal aunque no la uses actualmente
    void playerLevelUp();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void actualizarJuego();
    void generarBarril();

private:
    struct Barril {
        QPointF posicion;
        float velocidad;
        bool activo;

        Barril(QPointF pos, float vel) : posicion(pos), velocidad(vel), activo(true) {}

        QRectF getAreaColision() const {
            return QRectF(posicion.x() - 15, posicion.y() - 15, 30, 30);
        }

        bool estaFueraDePantalla() const {
            return posicion.y() > 768;
        }
    };

    void procesarColisiones();
    void limpiarBarriles();
    void dibujarJugador(QPainter &painter);
    void dibujarBarriles(QPainter &painter);
    void dibujarHUD(QPainter &painter);

    QTimer* timerJuego;
    QTimer* timerGeneracionBarriles;
    JugadorNivel2* jugador;
    QList<Barril> barriles;

    int tiempoTranscurrido = 0;
    int tiempoObjetivo = 90;
    int barrilesEsquivados = 0;
    bool enPausa = false;
};

#endif // NIVEL2_H
