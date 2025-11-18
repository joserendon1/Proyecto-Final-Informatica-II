#include "arma.h"
#include <QtMath>
#include <QRandomGenerator>
#include <QDebug>

Arma::Arma(Tipo tipoArma) :
    tipo(tipoArma),
    tiempoCooldownRestante(0),
    nivel(1)
{
    switch(tipo) {
    case ESPADA:
        nombre = "Espada Bizantina";
        danioBase = 15.0f;
        cooldownBase = 1200;
        color = QColor(255, 215, 0);
        break;
    case BALLESTA:
        nombre = "Ballesta";
        danioBase = 12.0f;
        cooldownBase = 1800;
        color = QColor(139, 69, 19);
        break;
    case ACEITE:
        nombre = "Aceite Hirviendo";
        danioBase = 25.0f;
        cooldownBase = 3000;
        color = QColor(255, 140, 0);
        break;
    case ARCO:
        nombre = "Arco Multidireccional";
        danioBase = 8.0f;
        cooldownBase = 1500;
        color = QColor(160, 82, 45);
        break;
    }

    danio = danioBase;
    cooldown = cooldownBase;
}

Arma::~Arma()
{
}

QString Arma::getNombre() const {
    return nombre + " Nv." + QString::number(nivel);
}

void Arma::subirNivel() {
    nivel++;
    danio = danioBase * (1.0f + (nivel - 1) * 0.15f);
    cooldown = cooldownBase * (1.0f - (nivel - 1) * 0.07f);
}

void Arma::activar(const QPointF& posicion, const QPointF& direccion)
{
    if(!puedeAtacar()) return;

    limpiarAtaques();

    switch(tipo) {
    case ESPADA:
        crearAtaqueEspada(posicion, direccion);
        break;
    case BALLESTA:
        crearAtaqueBallesta(posicion, direccion);
        break;
    case ACEITE:
        crearAtaqueAceite(posicion, direccion);
        break;
    case ARCO:
        crearAtaqueArco(posicion, direccion);
        break;
    }

    tiempoCooldownRestante = cooldown;
}

void Arma::crearAtaqueEspada(const QPointF& posicion, const QPointF& direccion)
{
    float longitud = 50.0f;
    float ancho = 25.0f;
    QPointF dirAtaque = direccion;

    if(dirAtaque.isNull()) {
        dirAtaque = QPointF(0, -1);
    }

    QRectF areaAtaque;
    float rotacion = 0.0f;

    if(qAbs(dirAtaque.x()) > qAbs(dirAtaque.y())) {
        if(dirAtaque.x() > 0) {
            areaAtaque = QRectF(posicion.x(), posicion.y() - ancho/2, longitud, ancho);
            rotacion = 0.0f;
        } else {
            areaAtaque = QRectF(posicion.x() - longitud, posicion.y() - ancho/2, longitud, ancho);
            rotacion = 180.0f;
        }
    } else {
        if(dirAtaque.y() > 0) {
            areaAtaque = QRectF(posicion.x() - ancho/2, posicion.y(), ancho, longitud);
            rotacion = 90.0f;
        } else {
            areaAtaque = QRectF(posicion.x() - ancho/2, posicion.y() - longitud, ancho, longitud);
            rotacion = -90.0f;
        }
    }

    areasAtaque.append(areaAtaque);

    AreaAtaqueSprite spriteAtaque;
    spriteAtaque.area = areaAtaque;
    spriteAtaque.tiempoVida = cooldown * 0.3f;
    spriteAtaque.frameActual = 0;
    spriteAtaque.spriteName = getSpriteSheetName();
    spriteAtaque.rotacion = rotacion;
    spriteAtaque.totalFrames = getTotalFrames();
    spriteAtaque.tiempoDesdeUltimoFrame = 0.0f;
    areasAtaqueSprites.append(spriteAtaque);
}

void Arma::crearAtaqueBallesta(const QPointF& posicion, const QPointF& direccion)
{
    int numProyectiles = 1;
    if(nivel >= 3) numProyectiles = 2;
    if(nivel >= 6) numProyectiles = 3;

    for(int i = 0; i < numProyectiles; i++) {
        QPointF dirAjustada = direccion;

        if(i > 0) {
            float angulo = (i % 2 == 0) ? 0.1f : -0.1f;
            QPointF rotacion(std::cos(angulo) * dirAjustada.x() - std::sin(angulo) * dirAjustada.y(),
                             std::sin(angulo) * dirAjustada.x() + std::cos(angulo) * dirAjustada.y());
            dirAjustada = rotacion;
        }

        proyectiles.append(posicion);
        direccionesProyectiles.append(dirAjustada);
        tiemposVidaProyectiles.append(1800.0f);

        ProyectilSprite proyectilSprite;
        proyectilSprite.posicion = posicion;
        proyectilSprite.direccion = dirAjustada;
        proyectilSprite.tiempoVida = 1800.0f;
        proyectilSprite.rotacion = std::atan2(dirAjustada.y(), dirAjustada.x()) * 180 / M_PI;
        proyectilSprite.frameActual = 0;
        proyectilesSprites.append(proyectilSprite);
    }
}

void Arma::crearAtaqueArco(const QPointF& posicion, const QPointF& direccion)
{
    Q_UNUSED(direccion);

    int numFlechas = 4;
    if(nivel >= 2) numFlechas = 5;
    if(nivel >= 4) numFlechas = 6;
    if(nivel >= 7) numFlechas = 8;

    for(int i = 0; i < numFlechas; i++) {
        float angulo = (2 * M_PI * i) / numFlechas;
        QPointF dirFlecha(std::cos(angulo), std::sin(angulo));

        proyectiles.append(posicion);
        direccionesProyectiles.append(dirFlecha);
        tiemposVidaProyectiles.append(1200.0f);

        ProyectilSprite flechaSprite;
        flechaSprite.posicion = posicion;
        flechaSprite.direccion = dirFlecha;
        flechaSprite.tiempoVida = 1200.0f;
        flechaSprite.rotacion = std::atan2(dirFlecha.y(), dirFlecha.x()) * 180 / M_PI;
        flechaSprite.frameActual = 0;
        proyectilesSprites.append(flechaSprite);
    }
}

void Arma::crearAtaqueAceite(const QPointF& posicion, const QPointF& direccion)
{
    QPointF dirLanzamiento = direccion;

    if(dirLanzamiento.isNull()) {
        dirLanzamiento = QPointF(0, -1);
    }

    int numManchas = 1 + (nivel / 2); // Más manchas con nivel más alto
    if(numManchas > 3) numManchas = 3;

    float distanciaBase = 80.0f;
    float dispersion = 40.0f; // Qué tan dispersas están las manchas

    for(int i = 0; i < numManchas; i++) {
        // Calcular posición de cada mancha
        QPointF offset;
        if(numManchas > 1) {
            // Patrón de dispersión
            float anguloDispersion = (i - (numManchas-1)/2.0f) * 0.3f;
            QPointF dirDispersada(
                std::cos(anguloDispersion) * dirLanzamiento.x() - std::sin(anguloDispersion) * dirLanzamiento.y(),
                std::sin(anguloDispersion) * dirLanzamiento.x() + std::cos(anguloDispersion) * dirLanzamiento.y()
                );
            offset = dirDispersada * distanciaBase;
        } else {
            offset = dirLanzamiento * distanciaBase;
        }

        // Añadir dispersión aleatoria
        offset += QPointF(
            (QRandomGenerator::global()->bounded(200) - 100) / 100.0f * dispersion,
            (QRandomGenerator::global()->bounded(200) - 100) / 100.0f * dispersion
            );

        QPointF posicionMancha = posicion + offset;
        float radio = (40.0f + nivel * 6.0f) * (0.8f + QRandomGenerator::global()->bounded(200) / 1000.0f);

        QRectF areaAtaque(posicionMancha.x() - radio, posicionMancha.y() - radio,
                          radio * 2, radio * 2);
        areasAtaque.append(areaAtaque);

        AreaAtaqueSprite aceiteSprite;
        aceiteSprite.area = areaAtaque;
        aceiteSprite.tiempoVida = cooldown * 0.5f;
        aceiteSprite.frameActual = QRandomGenerator::global()->bounded(9); // Frame aleatorio
        aceiteSprite.spriteName = getSpriteSheetName();
        aceiteSprite.rotacion = QRandomGenerator::global()->bounded(360); // Rotación aleatoria
        aceiteSprite.totalFrames = getTotalFrames();
        aceiteSprite.tiempoDesdeUltimoFrame = 0.0f;
        areasAtaqueSprites.append(aceiteSprite);
    }

    qDebug() << "Aceite lanzado -" << numManchas << "manchas creadas";
}

void Arma::actualizar(float deltaTime)
{
    if(tiempoCooldownRestante > 0) {
        tiempoCooldownRestante -= deltaTime;
        if(tiempoCooldownRestante < 0) tiempoCooldownRestante = 0;
    }

    if(tipo == BALLESTA || tipo == ARCO) {
        actualizarProyectiles(deltaTime);
    }

    actualizarSpritesAtaque(deltaTime);

    if(tipo == ESPADA || tipo == ACEITE) {
        if(tiempoCooldownRestante < cooldown * 0.15f) {
            limpiarAtaques();
        }
    }
}

void Arma::actualizarProyectiles(float deltaTime)
{
    float velocidadBase = 0.12f;
    float velocidad = velocidadBase * (1.0f + (nivel - 1) * 0.08f);

    // Actualizar proyectiles lógicos
    for(int i = 0; i < proyectiles.size(); i++) {
        proyectiles[i] += direccionesProyectiles[i] * velocidad * deltaTime;
        tiemposVidaProyectiles[i] -= deltaTime;
    }

    // Limpiar proyectiles expirados
    for(int i = proyectiles.size() - 1; i >= 0; i--) {
        if(tiemposVidaProyectiles[i] <= 0) {
            proyectiles.removeAt(i);
            direccionesProyectiles.removeAt(i);
            tiemposVidaProyectiles.removeAt(i);
        }
    }
}

void Arma::actualizarSpritesAtaque(float deltaTime)
{
    // Actualizar proyectiles con sprites
    for(int i = 0; i < proyectilesSprites.size(); i++) {
        proyectilesSprites[i].posicion += proyectilesSprites[i].direccion * 0.12f * deltaTime;
        proyectilesSprites[i].tiempoVida -= deltaTime;
        proyectilesSprites[i].frameActual = (proyectilesSprites[i].frameActual + 1) % 4;
    }

    // Actualizar áreas de ataque con sprites (ESPADA Y ACEITE)
    for(int i = 0; i < areasAtaqueSprites.size(); i++) {
        areasAtaqueSprites[i].tiempoVida -= deltaTime;
        areasAtaqueSprites[i].tiempoDesdeUltimoFrame += deltaTime; // NUEVO

        // Velocidad de animación controlada por tiempo
        float tiempoPorFrame = 80.0f; // 80ms por frame (12.5 FPS para la animación)

        if(areasAtaqueSprites[i].tiempoDesdeUltimoFrame >= tiempoPorFrame) {
            areasAtaqueSprites[i].frameActual =
                (areasAtaqueSprites[i].frameActual + 1) % areasAtaqueSprites[i].totalFrames;
            areasAtaqueSprites[i].tiempoDesdeUltimoFrame = 0;
        }
    }

    // Limpiar sprites expirados
    for(int i = proyectilesSprites.size() - 1; i >= 0; i--) {
        if(proyectilesSprites[i].tiempoVida <= 0) {
            proyectilesSprites.removeAt(i);
        }
    }

    for(int i = areasAtaqueSprites.size() - 1; i >= 0; i--) {
        if(areasAtaqueSprites[i].tiempoVida <= 0) {
            areasAtaqueSprites.removeAt(i);
        }
    }
}

QList<QRectF> Arma::getAreasAtaque() const
{
    QList<QRectF> todasLasAreas = areasAtaque;

    if(tipo == BALLESTA || tipo == ARCO) {
        float tamanoProyectil = 5.0f + nivel * 1.0f;
        for(const QPointF& proyectil : proyectiles) {
            todasLasAreas.append(QRectF(proyectil.x() - tamanoProyectil/2,
                                        proyectil.y() - tamanoProyectil/2,
                                        tamanoProyectil, tamanoProyectil));
        }
    }

    return todasLasAreas;
}

void Arma::limpiarAtaques()
{
    areasAtaque.clear();
}

QString Arma::getSpriteSheetName() const
{
    switch(tipo) {
    case ESPADA: return "sword_slash";
    case BALLESTA: return "projectile_arrow";
    case ARCO: return "projectile_arrow";
    case ACEITE: return "oil_effect";
    default: return "";
    }
}

int Arma::getTotalFrames() const
{
    switch(tipo) {
    case ESPADA: return 7;
    case BALLESTA: return 1;
    case ARCO: return 1;
    case ACEITE: return 9;
    default: return 1;
    }
}
