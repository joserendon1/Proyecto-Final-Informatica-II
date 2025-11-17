#ifndef ARMA_H
#define ARMA_H

#include <QObject>
#include <QList>
#include <QRectF>
#include <QPointF>

class Arma : public QObject
{
    Q_OBJECT

public:
    enum Tipo { ESPADA, BALLESTA, ACEITE, ARCO, LANZA, ESCUDO };

    Arma(Tipo tipo);
    ~Arma();

    void activar(const QPointF& posicion, const QPointF& direccion = QPointF(0, -1));
    void actualizar(float deltaTime);
    QList<QRectF> getAreasAtaque() const;
    bool puedeAtacar() const { return tiempoCooldownRestante <= 0; }

    Tipo getTipo() const { return tipo; }
    float getDanio() const { return danio; }
    void setDanio(float nuevoDanio) { danio = nuevoDanio; }
    int getCooldown() const { return cooldown; }
    void setCooldown(int nuevoCooldown) { cooldown = nuevoCooldown; }

private:
    void crearAtaqueEspada(const QPointF& posicion);
    void crearAtaqueBallesta(const QPointF& posicion, const QPointF& direccion);
    void actualizarProyectiles(float deltaTime);
    void limpiarAtaques();

    Tipo tipo;
    float danio;
    int cooldown;
    float tiempoCooldownRestante;

    QList<QRectF> areasAtaque;
    QList<QPointF> proyectiles;
    QList<QPointF> direccionesProyectiles;
    QList<float> tiemposVidaProyectiles;
};

#endif // ARMA_H
