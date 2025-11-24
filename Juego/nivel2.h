#ifndef NIVEL2_H
#define NIVEL2_H

#include "nivelbase.h"
#include "jugadornivel2.h"
#include "torre.h"
#include <QTimer>
#include <QList>
#include <QMouseEvent>

class Nivel2 : public NivelBase
{
    Q_OBJECT

public:
    explicit Nivel2(QWidget *parent = nullptr);
    ~Nivel2();

    // Implementación del método virtual puro
    void actualizarJuego(float deltaTime) override;

    // Override de métodos de NivelBase
    void iniciarNivel() override;
    void pausarNivel() override;
    void reanudarNivel() override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void generarOleada();
    void generarRecursos();

private:
    // Métodos específicos del nivel 2
    void inicializarRutas();
    void generarEnemigo();
    void procesarColisiones();
    void limpiarEnemigosMuertos();
    void actualizarTorres();

    // Métodos de UI
    void dibujarHUD(QPainter &painter);
    void dibujarRutas(QPainter &painter);
    void dibujarTorres(QPainter &painter);
    void dibujarInterfazConstruccion(QPainter &painter);
    void dibujarCastillo(QPainter &painter);

    // Sistema de construcción
    void procesarClickConstruccion(const QPointF& posicionMundo);
    bool esPosicionValidaParaTorre(const QPointF& posicion) const;
    Torre::Tipo tipoTorreSeleccionado = Torre::ARCO;

    // Estado del nivel 2
    QTimer* timerOleadas;
    QTimer* timerRecursos;
    QList<Enemigo*> enemigos;
    QList<QList<QPointF>> rutasEnemigas;

    int tiempoTranscurrido = 0;
    int tiempoObjetivo = 180; // 3 minutos
    int numeroOleada = 1;
    int recursosGenerados = 0;

    bool modoConstruccion = true;
    Torre* torreSeleccionada = nullptr;
};

#endif // NIVEL2_H
