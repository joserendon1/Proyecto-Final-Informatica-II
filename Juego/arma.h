#ifndef ARMA_H
#define ARMA_H

#include <QObject>
#include <QList>
#include <QRectF>
#include <QPointF>
#include <QColor>

class Arma : public QObject
{
    Q_OBJECT

public:
    enum Tipo { ESPADA, BALLESTA, ACEITE, ARCO };

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

    QColor getColor() const { return color; }
    QString getNombre() const;
    int getNivel() const { return nivel; }
    void subirNivel();

    // NUEVOS MÉTODOS PARA SPRITES
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

private:
    void crearAtaqueEspada(const QPointF& posicion, const QPointF& direccion);
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

    // NUEVAS VARIABLES PARA SPRITES
    QList<ProyectilSprite> proyectilesSprites;
    QList<AreaAtaqueSprite> areasAtaqueSprites;
};

#endif // ARMA_H
