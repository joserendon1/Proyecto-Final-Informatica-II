#ifndef NIVEL1_H
#define NIVEL1_H

#include <QWidget>
#include <QTimer>
#include <QKeyEvent>
#include <QPainter>
#include <QList>
#include <QRandomGenerator>
#include <QInputDialog>
#include <QDebug>
#include <QMessageBox>
#include <QDateTime>  // AGREGADO para deltaTime

#include "jugadornivel1.h"
#include "enemigo.h"
#include "mejora.h"

class Nivel1 : public QWidget
{
    Q_OBJECT

public:
    explicit Nivel1(QWidget *parent = nullptr);
    ~Nivel1();

    void iniciarNivel();
    void pausarNivel();
    void reanudarNivel();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void actualizarJuego();
    void generarOleada();

private:
    void procesarColisiones();
    void generarEnemigo();
    void dibujarHUD(QPainter &painter);
    void dibujarArmas(QPainter &painter);
    void limpiarEnemigosMuertos();
    void mostrarOpcionesMejoras();
    void inicializarMejoras();
    QList<Mejora> generarOpcionesMejoras(int cantidad = 3);
    void aplicarMejora(const Mejora& mejora);
    void resetearTeclas();

    JugadorNivel1 *jugador;
    QList<Enemigo*> enemigos;

    QTimer *timerJuego;
    QTimer *timerOleadas;

    bool teclas[4]; // W, A, S, D
    int tiempoTranscurrido; // en segundos
    int tiempoObjetivo; // 2 minutos = 120 segundos
    int numeroOleada;

    // Configuración de oleadas
    int enemigosPorOleada;
    int frecuenciaGeneracion;

    // Estado de mejoras
    bool mostrandoMejoras;
    QList<Mejora> todasLasMejoras;

    qint64 tiempoUltimoFrame; // Para calcular deltaTime
};

#endif // NIVEL1_H
