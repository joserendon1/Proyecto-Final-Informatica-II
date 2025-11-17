#ifndef ARMA_H
#define ARMA_H

#include <QObject>
#include <QTimer>
#include <QList>
#include <QRectF>
#include <QPointF>

class Arma : public QObject
{
    Q_OBJECT

public:
    enum Tipo {
        ESPADA,
        BALLESTA,
        ACEITE  // Para futuras expansiones
    };

    Arma(Tipo tipo);
    ~Arma();

    void activar(const QPointF& posicionJugador);
    void actualizar();
    bool puedeAtacar() const { return listaParaAtacar; }

    QList<QRectF> getAreasAtaque() const;
    Tipo getTipo() const { return tipo; }

    // Métodos para mejoras
    void setDanio(float nuevoDanio) { danio = nuevoDanio; }
    void setCooldown(int nuevoCooldown) { cooldown = nuevoCooldown; }

private slots:
    void resetCooldown();

private:
    void crearAtaqueEspada(const QPointF& posicion);
    void crearAtaqueBallesta(const QPointF& posicion);
    void actualizarProyectiles();

    Tipo tipo;
    QTimer* timerCooldown;
    QList<QRectF> areasAtaque;
    QList<QPointF> proyectiles; // Para ballesta
    QList<QPointF> direccionesProyectiles;
    bool listaParaAtacar;
    float danio;
    int cooldown;
};

#endif // ARMA_H
