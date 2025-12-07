#ifndef ARMA_H
#define ARMA_H

#include <QObject>
#include <QList>
#include <QRectF>
#include <QPointF>
#include <QColor>
#include <enemigo.h>

class Arma : public QObject
{
    Q_OBJECT

public:
    enum Tipo { BALLESTA, ACEITE, ARCO, CATAPULTA, MAGICA };

    Arma(Tipo tipo);
    virtual ~Arma();

    virtual void activar(const QPointF& posicion, const QPointF& direccion = QPointF(0, -1));
    virtual void actualizar(float deltaTime);
    QList<QRectF> getAreasAtaque() const;
    bool puedeAtacar() const { return tiempoCooldownRestante <= 0; }

    Tipo getTipo() const { return tipo; }
    float getDanio() const { return danio; }
    void setDanio(float nuevoDanio) { danio = nuevoDanio; }
    int getCooldown() const { return cooldown; }
    void setCooldown(int nuevoCooldown) { cooldown = nuevoCooldown; }
    void setReferenciasJugador(QPointF* pos, QPointF* dir) {posicionJugador = pos; direccionJugador = dir;}
    void setEnemigosCercanos(const QList<Enemigo*>& enemigos);
    QPointF calcularDireccionHaciaEnemigoCercano(const QPointF& posicionJugador);

    QColor getColor() const { return color; }
    QString getNombre() const;
    int getNivel() const { return nivel; }
    void subirNivel();

    struct ProyectilSprite {
        QPointF posicion;
        QPointF direccion;
        float tiempoVida;
        float rotacion;
        int frameActual;
    };

    struct AreaAtaqueSprite {
        QRectF area;
        float tiempoVida;
        int frameActual;
        QString spriteName;
        float rotacion;
        int totalFrames;
        float tiempoDesdeUltimoFrame;
    };

    QList<ProyectilSprite> getProyectilesSprites() const { return proyectilesSprites; }
    QList<AreaAtaqueSprite> getAreasAtaqueSprites() const { return areasAtaqueSprites; }
    QString getSpriteSheetName() const;
    int getTotalFrames() const;

protected:
    static constexpr float DURACION_ATAQUE_ACEITE = 1400.0f;

    void crearAtaqueBallesta(const QPointF& posicion, const QPointF& direccion);
    void crearAtaqueArco(const QPointF& posicion, const QPointF& direccion);
    void crearAtaqueAceite(const QPointF& posicion, const QPointF& direccion);
    void actualizarProyectiles(float deltaTime);
    void actualizarSpritesAtaque(float deltaTime);
    void limpiarAtaques();

    Tipo tipo;
    float danio;
    int cooldown;
    float tiempoCooldownRestante;

    int nivel;
    float danioBase;
    int cooldownBase;

    QColor color;
    QString nombre;

    QList<QRectF> areasAtaque;
    QList<QPointF> proyectiles;
    QList<QPointF> direccionesProyectiles;
    QList<float> tiemposVidaProyectiles;

    QList<ProyectilSprite> proyectilesSprites;
    QList<AreaAtaqueSprite> areasAtaqueSprites;

    QPointF* posicionJugador;
    QPointF* direccionJugador;

    QList<Enemigo*> enemigosCercanos;
    float rangoDeteccion;

private:
    void crearAtaqueEspada(const QPointF& posicion, const QPointF& direccion);
};

#endif // ARMA_H
