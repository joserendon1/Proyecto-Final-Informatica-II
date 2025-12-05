#include "nivel3.h"
#include "audiomanager.h"
#include "spritemanager.h"
#include "uimanager.h"
#include <QPainter>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QDebug>


Nivel3::Nivel3(QWidget *parent) : NivelBase(parent)
    , velocidadScroll(3.0f)
    , distanciaRecorrida(0)
    , tiempoObjetivo(60.0f)
    , spawnRate(2.5f)  // Más lento al inicio
    , tiempoDesdeUltimoSpawn(0)
    , frameAnimacion(0)      // MOVER ARRIBA DE juegoActivo
    , tiempoAnimacion(0)
    , jugadorN3(nullptr)
    , nivelCompletado(false)
    , juegoActivo(true)      // MOVER DESPUÉS DE frameAnimacion
{
    setFocusPolicy(Qt::StrongFocus);

    // SOLO 2 OBSTÁCULOS DISPONIBLES
    milestonesObstaculos = {
        {0.0f, 1},      // Desde el inicio: obstacle1 disponible (alto)
        {1000.0f, 3}    // A 1000px: agregar obstacle3 (bajo)
    };

    setupNivel();
}

Nivel3::~Nivel3()
{
}

QSize Nivel3::obtenerDimensionesSprite(int tipoObstaculo)
{
    switch (tipoObstaculo) {
    case 1: return QSize(64, 128);   // obstacle1 (alto)
    case 3: return QSize(64, 64);    // obstacle3 (bajo)
    default: return QSize(64, 128);
    }
}

QSize Nivel3::obtenerDimensionesHitbox(int tipoObstaculo)
{
    // Hitbox más pequeña que el sprite (ajusta estos valores según necesites)
    switch (tipoObstaculo) {
    case 1: return QSize(50, 100);   // 78% del tamaño original
    case 3: return QSize(50, 50);    // 78% del tamaño original
    default: return QSize(50, 100);
    }
}

int Nivel3::obtenerAjusteY(int tipoObstaculo)
{
    switch (tipoObstaculo) {
    case 1: return AJUSTE_OBSTACULO1;  // 30
    case 3: return AJUSTE_OBSTACULO3;  // 20
    default: return 30;
    }
}

void Nivel3::setupNivel()
{
    tamanoVista = QSize(960, 720);
    setFixedSize(tamanoVista);

    nivelCompletado = false;
    juegoActivo = true;
    tiempoDesdeUltimoSpawn = 0;
    frameAnimacion = 0;
    tiempoAnimacion = 0;

    SpriteManager::getInstance().preloadGameSprites();

    jugadorN3 = new JugadorNivel3();
    jugador = jugadorN3;

    // Posición del jugador
    jugadorN3->setPosicion(QPointF(100, 450));

    posicionCamara = QPointF(0, 0);
    obstaculos.clear();
    spriteRects.clear();
    tiposObstaculos.clear();
    powerUps.clear();

    AudioManager::getInstance().loadSounds();

    if (timerJuego) {
        timerJuego->start(16);
    }

    timerNivel.start();
    qDebug() << "🎮 Nivel 3 iniciado - Solo obstacle1 y obstacle3";
}

void Nivel3::iniciarNivel()
{
    juegoActivo = true;
    if (timerJuego && !timerJuego->isActive()) {
        timerJuego->start(16);
    }
    AudioManager::getInstance().playBackgroundMusic();
}

void Nivel3::pausarNivel()
{
    NivelBase::pausarNivel();
    AudioManager::getInstance().stopAllSounds();
}

void Nivel3::reanudarNivel()
{
    NivelBase::reanudarNivel();
    AudioManager::getInstance().playBackgroundMusic();
}

void Nivel3::actualizarJuego(float deltaTime)
{
    if (!juegoActivo) return;

    jugadorN3->actualizar(deltaTime);

    // DEBUG ocasional
    static int debugCounter = 0;
    if (debugCounter++ % 120 == 0) {
        qDebug() << "🎯 Jugador - Pos Y:" << jugadorN3->getPosicion().y()
            << "Saltando:" << jugadorN3->estaSaltando
            << "Agachado:" << jugadorN3->estaAgachado;
    }

    actualizarAnimacion(deltaTime);
    actualizarCamaraAutoScroll();

    tiempoDesdeUltimoSpawn += deltaTime;
    if (tiempoDesdeUltimoSpawn > spawnRate * 1000) {
        generarObstaculos();
        tiempoDesdeUltimoSpawn = 0;
    }

    verificarColisiones();

    if (timerNivel.elapsed() / 1000.0f >= tiempoObjetivo) {
        nivelCompletado = true;
        juegoActivo = false;
        emit levelCompleted();
        AudioManager::getInstance().playLevelUp();
    }

    update();
}

void Nivel3::actualizarAnimacion(float deltaTime)
{
    tiempoAnimacion += deltaTime;

    if (tiempoAnimacion > 100) {
        if (jugadorN3->estaSaltando || jugadorN3->estaAgachado) {
            frameAnimacion = 0;
        } else {
            frameAnimacion = (frameAnimacion + 1) % 6;
        }
        tiempoAnimacion = 0;
    }
}

void Nivel3::actualizarCamaraAutoScroll()
{
    float dificultad = qMin(distanciaRecorrida / 3000.0f, 1.0f);
    velocidadScroll = 2.5f + dificultad * 1.5f;
    spawnRate = 2.5f - dificultad * 0.5f;

    posicionCamara.setX(posicionCamara.x() + velocidadScroll);
    distanciaRecorrida += velocidadScroll;
}

QList<int> Nivel3::getObstaculosDisponibles() {
    QList<int> disponibles = {1}; // obstacle1 siempre disponible

    for (const auto& milestone : milestonesObstaculos) {
        if (distanciaRecorrida >= milestone.first) {
            if (!disponibles.contains(milestone.second)) {
                disponibles.append(milestone.second);
            }
        }
    }

    qDebug() << "📊 Distancia:" << distanciaRecorrida
             << "Obstáculos disponibles:" << disponibles;
    return disponibles;
}

void Nivel3::generarObstaculos()
{
    QRandomGenerator* random = QRandomGenerator::global();

    bool usarPatron = (distanciaRecorrida > 500) && (random->bounded(100) < 30);

    if (usarPatron) {
        int patron = random->bounded(2);
        generarPatronObstaculos(patron);
        qDebug() << "🎯 Patrón de obstáculos:" << patron;
    } else {
        generarObstaculosAleatorios();
    }
}

void Nivel3::generarPatronObstaculos(int tipoPatron)
{
    QList<int> obstaculosDisponibles = getObstaculosDisponibles();
    float baseX = posicionCamara.x() + width() + 200;

    switch (tipoPatron) {
    case 0: // Triple salto (todos obstacle3 si está disponible, sino obstacle1)
    {
        int tipoUsar = obstaculosDisponibles.contains(3) ? 3 : 1;
        QSize spriteSize = obtenerDimensionesSprite(tipoUsar);
        QSize hitboxSize = obtenerDimensionesHitbox(tipoUsar);
        int ajusteY = obtenerAjusteY(tipoUsar);

        for (int i = 0; i < 3; i++) {
            float x = baseX + (i * 200); // 200px entre cada uno

            // Rectángulo del sprite (para dibujar)
            QRectF spriteRect(x, SUELO_Y - spriteSize.height() + ajusteY,
                              spriteSize.width(), spriteSize.height());

            // Rectángulo de hitbox (más pequeño, centrado)
            float hitboxX = x + (spriteSize.width() - hitboxSize.width()) / 2;
            float hitboxY = SUELO_Y - spriteSize.height() + ajusteY +
                            (spriteSize.height() - hitboxSize.height());

            QRectF hitboxRect(hitboxX, hitboxY, hitboxSize.width(), hitboxSize.height());

            spriteRects.append(spriteRect);
            obstaculos.append(hitboxRect);
            tiposObstaculos.append(tipoUsar);
        }
    }
    break;

    case 1: // Alto-Bajo-Alto
    {
        // Asegurar que tenemos ambos tipos disponibles
        int tipoAlto = 1;  // obstacle1
        int tipoBajo = obstaculosDisponibles.contains(3) ? 3 : 1;

        QSize spriteAlto = obtenerDimensionesSprite(tipoAlto);
        QSize hitboxAlto = obtenerDimensionesHitbox(tipoAlto);
        int ajusteAlto = obtenerAjusteY(tipoAlto);

        QSize spriteBajo = obtenerDimensionesSprite(tipoBajo);
        QSize hitboxBajo = obtenerDimensionesHitbox(tipoBajo);
        int ajusteBajo = obtenerAjusteY(tipoBajo);

        // Patrón: Alto (200px) -> Bajo (200px) -> Alto (200px)
        for (int i = 0; i < 3; i++) {
            float x = baseX + (i * 200);
            int tipoActual = (i % 2 == 0) ? tipoAlto : tipoBajo;
            QSize spriteSize = (i % 2 == 0) ? spriteAlto : spriteBajo;
            QSize hitboxSize = (i % 2 == 0) ? hitboxAlto : hitboxBajo;
            int ajusteActual = (i % 2 == 0) ? ajusteAlto : ajusteBajo;

            // Rectángulo del sprite
            QRectF spriteRect(x, SUELO_Y - spriteSize.height() + ajusteActual,
                              spriteSize.width(), spriteSize.height());

            // Rectángulo de hitbox
            float hitboxX = x + (spriteSize.width() - hitboxSize.width()) / 2;
            float hitboxY = SUELO_Y - spriteSize.height() + ajusteActual +
                            (spriteSize.height() - hitboxSize.height());

            QRectF hitboxRect(hitboxX, hitboxY, hitboxSize.width(), hitboxSize.height());

            spriteRects.append(spriteRect);
            obstaculos.append(hitboxRect);
            tiposObstaculos.append(tipoActual);
        }
    }
    break;
    }
}

void Nivel3::generarObstaculosAleatorios()
{
    QRandomGenerator* random = QRandomGenerator::global();
    QList<int> obstaculosDisponibles = getObstaculosDisponibles();

    // Solo 1 obstáculo a la vez para hacerlo más fácil
    int numObstaculos = 1;

    // Solo después de mucha distancia, posibilidad de 2 obstáculos
    if (distanciaRecorrida > 2000 && random->bounded(100) < 20) {
        numObstaculos = 2;
    }

    float ultimaPosicionX = posicionCamara.x() + width() + 200;

    for (int i = 0; i < numObstaculos; i++) {
        // Separación generosa
        int separacion = random->bounded(SEPARACION_MINIMA, SEPARACION_MAXIMA);
        float x = ultimaPosicionX + separacion;

        // Elegir tipo de obstáculo
        int indexTipo = random->bounded(obstaculosDisponibles.size());
        int tipoObstaculo = obstaculosDisponibles[indexTipo];

        // Obtener dimensiones
        QSize spriteSize = obtenerDimensionesSprite(tipoObstaculo);
        QSize hitboxSize = obtenerDimensionesHitbox(tipoObstaculo);
        int ajusteY = obtenerAjusteY(tipoObstaculo);

        // Rectángulo del sprite (para dibujar)
        QRectF spriteRect(x, SUELO_Y - spriteSize.height() + ajusteY,
                          spriteSize.width(), spriteSize.height());

        // Rectángulo de hitbox (más pequeño, centrado)
        float hitboxX = x + (spriteSize.width() - hitboxSize.width()) / 2;
        float hitboxY = SUELO_Y - spriteSize.height() + ajusteY +
                        (spriteSize.height() - hitboxSize.height());

        QRectF hitboxRect(hitboxX, hitboxY, hitboxSize.width(), hitboxSize.height());

        spriteRects.append(spriteRect);
        obstaculos.append(hitboxRect);
        tiposObstaculos.append(tipoObstaculo);

        // Actualizar posición para el siguiente obstáculo
        ultimaPosicionX = x + spriteSize.width() + 100;

        qDebug() << "🎯 Generado obstáculo tipo" << tipoObstaculo
                 << "Sprite:" << spriteSize.width() << "x" << spriteSize.height()
                 << "Hitbox:" << hitboxSize.width() << "x" << hitboxSize.height()
                 << "Pos Y:" << spriteRect.y();
    }
}

void Nivel3::verificarColisiones()
{
    float jugadorMundoX = posicionCamara.x() + 100;
    float jugadorMundoY = jugadorN3->getPosicion().y();

    QRectF areaJugador = jugadorN3->getAreaColision();
    areaJugador.moveTo(jugadorMundoX - areaJugador.width()/2,
                       jugadorMundoY - areaJugador.height()/2);

    for (int i = obstaculos.size() - 1; i >= 0; i--) {
        QRectF obstaculo = obstaculos[i];

        if (areaJugador.intersects(obstaculo)) {
            qDebug() << "💥 COLISIÓN con obstáculo tipo" << tiposObstaculos[i];
            jugadorN3->setVida(jugadorN3->getVida() - 1);
            AudioManager::getInstance().playPlayerHurt();

            obstaculos.removeAt(i);
            spriteRects.removeAt(i); // También remover el rectángulo de sprite
            tiposObstaculos.removeAt(i);

            if (jugadorN3->getVida() <= 0) {
                juegoActivo = false;
                emit gameOver();
                AudioManager::getInstance().playPlayerHurt();
            }
        }
    }

    // Limpiar obstáculos que ya pasaron
    for (int i = obstaculos.size() - 1; i >= 0; i--) {
        if (obstaculos[i].right() < posicionCamara.x() - 200) {
            obstaculos.removeAt(i);
            spriteRects.removeAt(i);
            tiposObstaculos.removeAt(i);
        }
    }
}

void Nivel3::keyPressEvent(QKeyEvent *event)
{
    switch(event->key()) {
    case Qt::Key_Space:
    case Qt::Key_W:
        if (!jugadorN3->estaSaltando && !jugadorN3->estaAgachado) {
            jugadorN3->saltar();
            AudioManager::getInstance().playPlayerMove();
            qDebug() << "🔼 Tecla Salto presionada";
        }
        break;
    case Qt::Key_S:
        if (jugadorN3->estaSaltando) {
            // Si está saltando, cancelar salto para caer rápido
            jugadorN3->cancelarSalto();
            AudioManager::getInstance().playPlayerMove();
            qDebug() << "🔽 Tecla S presionada durante salto - Cancelando salto";
        } else if (!jugadorN3->estaSaltando) {
            // Si está en el suelo, agacharse normalmente
            jugadorN3->agacharse();
            AudioManager::getInstance().playPlayerMove();
            qDebug() << "🔽 Tecla S presionada en suelo - Agachándose";
        }
        break;
    case Qt::Key_P:
        pausarNivel();
        qDebug() << "⏸️ Juego pausado";
        break;
    case Qt::Key_R:
        reanudarNivel();
        qDebug() << "▶️ Juego reanudado";
        break;
    default:
        // Otras teclas
        break;
    }
}

void Nivel3::keyReleaseEvent(QKeyEvent *event)
{
    switch(event->key()) {
    case Qt::Key_S:
        if (jugadorN3->estaAgachado && !jugadorN3->estaSaltando) {
            jugadorN3->levantarse();
            qDebug() << "Tecla S liberada - Levantándose";
        }
        break;
    default:
        break;
    }
}

void Nivel3::paintEvent(QPaintEvent *event){
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Fondo del cielo
    painter.fillRect(rect(), QColor(135, 206, 235));

    // Dibujar suelo con sprite
    dibujarSueloConSprite(painter);

    // Dibujar obstáculos
    dibujarObstaculosConSprites(painter);

    // Power-ups
    painter.setBrush(QBrush(QColor(255, 255, 0)));
    painter.setPen(QPen(Qt::black, 2));
    for (const QRectF& powerUp : powerUps) {
        QRectF powerUpVista = powerUp.translated(-posicionCamara);
        if (estaEnVista(powerUpVista)) {
            painter.drawEllipse(powerUpVista);
        }
    }

    // Dibujar jugador con sprite
    dibujarJugador(painter);

    // HUD mejorado
    dibujarHUD(painter);

    if (nivelCompletado) {
        painter.fillRect(rect(), QColor(0, 0, 0, 150));
        UIManager::getInstance().drawText(painter, "¡NIVEL COMPLETADO!",
                                          width()/2 - 150, height()/2, 2.0f);
    }
}

void Nivel3::dibujarSueloConSprite(QPainter &painter)
{
    QPixmap sueloSprite = SpriteManager::getInstance().getSprite("ground2");

    if (!sueloSprite.isNull()) {
        int sueloY = 550;

        int anchoVista = width();
        int repeticiones = (anchoVista / sueloSprite.width()) + 2;

        for (int i = -1; i < repeticiones; i++) {
            QRectF destino(i * sueloSprite.width() - fmod(posicionCamara.x(), sueloSprite.width()),
                           sueloY,
                           sueloSprite.width(),
                           sueloSprite.height());
            painter.drawPixmap(destino, sueloSprite, sueloSprite.rect());
        }
    } else {
        painter.fillRect(0, 550, width(), height() - 550, QColor(100, 200, 100));
    }
}

void Nivel3::dibujarObstaculosConSprites(QPainter &painter)
{
    for (int i = 0; i < spriteRects.size(); i++) {
        QRectF spriteRect = spriteRects[i];
        QRectF spriteVista = spriteRect.translated(-posicionCamara);

        if (spriteVista.right() > 0 && spriteVista.left() < width()) {
            QString spriteName;
            switch (tiposObstaculos[i]) {
            case 1: spriteName = "obstacle1"; break;
            case 3: spriteName = "obstacle3"; break;
            default: spriteName = "obstacle1"; break;
            }

            QPixmap obstaculoSprite = SpriteManager::getInstance().getSprite(spriteName);

            if (!obstaculoSprite.isNull()) {
                // Dibujar el sprite completo
                painter.drawPixmap(spriteVista, obstaculoSprite, obstaculoSprite.rect());
            } else {
                // Fallback: colores diferentes para cada tipo
                if (tiposObstaculos[i] == 1) {
                    painter.setBrush(QBrush(QColor(150, 75, 0))); // Marrón para obstacle1
                } else {
                    painter.setBrush(QBrush(QColor(100, 100, 100))); // Gris para obstacle3
                }
                painter.setPen(QPen(Qt::black, 2));
                painter.drawRect(spriteVista);
            }
        }
    }
}

void Nivel3::dibujarJugador(QPainter &painter)
{
    float jugadorY = jugadorN3->getPosicion().y();

    QString spriteName;
    int frameIndex = frameAnimacion;
    int frameWidth, frameHeight;
    QSize displaySize(80, 100);

    if (jugadorN3->estaSaltando) {
        spriteName = "player_move";
        frameWidth = 1152 / 6;
        frameHeight = 192;
        frameIndex = 0;
    } else if (jugadorN3->estaAgachado) {
        spriteName = "player_idle";
        frameWidth = 1536 / 8;
        frameHeight = 192;
        displaySize = QSize(80, 70);
        frameIndex = 0;
    } else {
        spriteName = "player_move";
        frameWidth = 1152 / 6;
        frameHeight = 192;
        frameIndex = frameAnimacion % 6;
    }

    QPixmap spriteSheet = SpriteManager::getInstance().getSprite(spriteName);

    if(!spriteSheet.isNull()) {
        QRect frameRect(frameIndex * frameWidth, 0, frameWidth, frameHeight);
        QPixmap frame = spriteSheet.copy(frameRect);

        QRectF displayRect(100 - displaySize.width()/2,
                           jugadorY - displaySize.height()/2,
                           displaySize.width(),
                           displaySize.height());
        painter.drawPixmap(displayRect, frame, frame.rect());

    } else {
        painter.setBrush(QBrush(QColor(0, 100, 200)));
        painter.setPen(QPen(Qt::white, 3));
        QRectF jugadorRect(80, jugadorY - 25, 40, 50);
        painter.drawRect(jugadorRect);
    }
}

void Nivel3::dibujarHUD(QPainter &painter)
{
    float tiempoRestante = tiempoObjetivo - (timerNivel.elapsed() / 1000.0f);
    if (tiempoRestante < 0) tiempoRestante = 0;

    // Fondo del HUD
    QPixmap hudBg = UIManager::getInstance().getHudPanel();
    if (!hudBg.isNull()) {
        painter.drawPixmap(10, 10, hudBg.scaled(200, 80));
    } else {
        painter.setBrush(QBrush(QColor(0, 0, 0, 150)));
        painter.setPen(Qt::NoPen);
        painter.drawRect(10, 10, 200, 80);
    }

    // Información del HUD
    UIManager::getInstance().drawText(painter,
                                      QString("Vidas: %1").arg((int)jugadorN3->getVida()), 30, 35);
    UIManager::getInstance().drawText(painter,
                                      QString("Tiempo: %1s").arg((int)tiempoRestante), 30, 55);
    UIManager::getInstance().drawText(painter,
                                      QString("Distancia: %1m").arg((int)(distanciaRecorrida / 10)), 30, 75);

    // Barra de tiempo
    float progresoTiempo = tiempoRestante / tiempoObjetivo;
    QPixmap ribbon = UIManager::getInstance().getRibbonRed();
    if (!ribbon.isNull()) {
        QPixmap ribbonEscalado = ribbon.scaled(200, 30);
        int anchoVisible = static_cast<int>(200 * progresoTiempo);
        if (anchoVisible > 0) {
            QPixmap ribbonRecortado = ribbonEscalado.copy(0, 0, anchoVisible, 30);
            painter.drawPixmap(220, 15, ribbonRecortado);
        }
    } else {
        painter.setBrush(QBrush(QColor(255, 0, 0, 180)));
        painter.setPen(Qt::NoPen);
        painter.drawRect(220, 15, static_cast<int>(200 * progresoTiempo), 20);
    }

    // Controles
    UIManager::getInstance().drawText(painter,
                                      "ESPACIO: Saltar   S: Agacharse",
                                      width()/2 - 150, height() - 30);
}
