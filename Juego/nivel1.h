#ifndef NIVEL1_H
#define NIVEL1_H

#include "nivelbase.h"
#include "jugadornivel1.h"
#include "enemigo.h"
#include "mejora.h"
#include <QTimer>
#include <QList>

class Nivel1 : public NivelBase
{
    Q_OBJECT

public:
    explicit Nivel1(QWidget *parent = nullptr);
    ~Nivel1();

    // Implementación del método virtual puro
    void actualizarJuego(float deltaTime) override;

    // Override de métodos de NivelBase
    void iniciarNivel() override;
    void pausarNivel() override;
    void reanudarNivel() override;
    void resetearTeclas();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void generarOleada();
    void onMejoraSeleccionada();

private:
    // Métodos específicos del nivel 1
    void inicializarMapaGrande();
    void inicializarMejoras();
    void generarEnemigo();
    void procesarColisiones();
    void limpiarEnemigosMuertos();
    void verificarYCorregirLimitesMapa(Entidad* entidad);
    bool verificarColisionMapa(const QRectF& area) const;

    // Métodos de UI
    void dibujarHUD(QPainter &painter);
    void dibujarArmas(QPainter &painter);
    void dibujarAtaqueAceite(QPainter &painter, Arma* arma, const Arma::AreaAtaqueSprite& areaSprite, const QRectF& areaRelativa);
    void dibujarBarraVidaEnemigo(QPainter &painter, Enemigo *enemigo, const QPointF &posicionRelativa);
    void dibujarMenuMejoras(QPainter &painter);
    void dibujarEntidadConSprite(QPainter &painter, const QPointF &posicionRelativa, const QString &spriteName, const QSize &displaySize, int frameWidth, int frameHeight, int currentFrame);

    // Sistema de mejoras
    QList<Mejora> generarOpcionesMejoras(int cantidad);
    void mostrarOpcionesMejoras();
    void aplicarMejora(const Mejora& mejora);
    void procesarSeleccionMejora(int opcion);

    // Estado del nivel 1
    QTimer* timerOleadas;
    QList<Enemigo*> enemigos;
    QList<Mejora> todasLasMejoras;
    QList<Mejora> opcionesMejorasActuales;

    int tiempoTranscurrido = 0;
    int tiempoObjetivo = 120;
    int numeroOleada = 1;
    int enemigosPorOleada = 4;
    int frecuenciaGeneracion = 3500;

    bool mostrandoMejoras = false;
    int opcionSeleccionada = 0;
};

#endif // NIVEL1_H
