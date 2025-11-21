#ifndef NIVEL1_H
#define NIVEL1_H

#include <QWidget>
#include <QTimer>
#include <QKeyEvent>
#include <QPainter>
#include <QList>
#include <QRandomGenerator>
#include <QDebug>
#include <QMessageBox>
#include <QDateTime>

#include "jugadornivel1.h"
#include "enemigo.h"
#include "mejora.h"
#include "mapa.h"

class Nivel1 : public QWidget
{
    Q_OBJECT

public:
    explicit Nivel1(QWidget *parent = nullptr);
    ~Nivel1();

    void iniciarNivel();
    void pausarNivel();
    void reanudarNivel();
    Mapa* getMapa() const { return mapa; }

signals:
    void gamePaused();
    void gameResumed();
    void playerLevelUp();
    void gameOver();
    void levelCompleted();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void actualizarJuego();
    void generarOleada();
    void onMejoraSeleccionada();

private:
    // MÉTODOS DE COLISIONES Y MAPA
    bool verificarColisionMapa(const QRectF& area) const;
    void inicializarMapaGrande();
    void verificarYCorregirLimitesMapa(Entidad* entidad);

    // MÉTODOS DE CÁMARA
    void actualizarCamara();
    QRectF getVistaCamara() const;
    bool estaEnVista(const QPointF& posicion) const;
    bool estaEnVista(const QRectF& area) const;

    // MÉTODOS DEL JUEGO
    void procesarColisiones();
    void generarEnemigo();
    void dibujarBarraVidaEnemigo(QPainter &painter, Enemigo *enemigo, const QPointF &posicion);
    void dibujarHUD(QPainter &painter);
    void dibujarArmas(QPainter &painter);
    void dibujarAtaqueAceite(QPainter &painter, Arma* arma, const Arma::AreaAtaqueSprite& areaSprite, const QRectF& areaRelativa);
    void dibujarEntidadConSprite(QPainter &painter, const QPointF &posicionRelativa,
                                 const QString &spriteName, const QSize &displaySize,
                                 int frameWidth, int frameHeight, int currentFrame);
    void limpiarEnemigosMuertos();

    void mostrarOpcionesMejoras();
    void inicializarMejoras();
    QList<Mejora> generarOpcionesMejoras(int cantidad = 3);
    void aplicarMejora(const Mejora& mejora);
    void resetearTeclas();

    // VARIABLES MIEMBRO
    JugadorNivel1 *jugador;
    QList<Enemigo*> enemigos;
    Mapa *mapa;

    QTimer *timerJuego;
    QTimer *timerOleadas;

    bool teclas[4];
    int tiempoTranscurrido;
    int tiempoObjetivo;
    int numeroOleada;

    // Configuración de oleadas
    int enemigosPorOleada;
    int frecuenciaGeneracion;

    // Estado de mejoras
    bool mostrandoMejoras;
    QList<Mejora> todasLasMejoras;
    QList<Mejora> opcionesMejorasActuales;
    int opcionSeleccionada;

    void dibujarMenuMejoras(QPainter &painter);
    void procesarSeleccionMejora(int opcion);

    qint64 tiempoUltimoFrame;

    // Variables de cámara
    QPointF posicionCamara;
    QSize tamanoVista;

    // Límites de spawn
    float margenSpawnExterior;
    float margenSpawnInterior;
};

#endif // NIVEL1_H
