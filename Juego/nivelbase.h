#ifndef NIVELBASE_H
#define NIVELBASE_H

#include "jugadorbase.h"
#include "mapa.h"
#include <QWidget>
#include <QTimer>
#include <vector>

class NivelBase : public QWidget
{
    Q_OBJECT

public:
    explicit NivelBase(QWidget *parent = nullptr);
    virtual ~NivelBase();
    virtual void iniciarNivel();
    virtual void pausarNivel();
    virtual void reanudarNivel();
    virtual void actualizarJuego(float deltaTime) = 0;
    void actualizarCamara();
    QRectF getVistaCamara() const;
    bool estaEnVista(const QPointF& posicion) const;
    bool estaEnVista(const QRectF& area) const;
    void resetearTeclas();
    JugadorBase* getJugador() const { return jugador; }
    Mapa* getMapa() const { return mapa; }
    QPointF getPosicionCamara() const { return posicionCamara; }

signals:
    void gamePaused();
    void gameResumed();
    void gameOver();
    void levelCompleted();
    void playerLevelUp();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    JugadorBase* jugador;
    Mapa* mapa;
    QTimer* timerJuego;
    QSize tamanoVista;
    QPointF posicionCamara;
    std::vector<bool> teclas;

private slots:
    void onTimerTimeout();

private:
    void setupNivelBase();
    void setupTimer();
    qint64 tiempoUltimoFrame;
};

#endif // NIVELBASE_H
