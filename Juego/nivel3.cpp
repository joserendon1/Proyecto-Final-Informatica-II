#include "nivel3.h"
#include "audiomanager.h"
#include "spritemanager.h"
#include "uimanager.h"
#include <QPainter>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QDebug>
#include <algorithm>

Nivel3::Nivel3(QWidget *parent) : NivelBase(parent)
    , velocidadScroll(3.0f)
    , distanciaRecorrida(0)
    , tiempoObjetivo(60.0f)
    , spawnRate(2.0f)
    , tiempoDesdeUltimoSpawn(0)
    , jugadorN3(nullptr)
    , nivelCompletado(false)
    , juegoActivo(true)
    , frameAnimacion(0)
    , tiempoAnimacion(0)
{
    setFocusPolicy(Qt::StrongFocus);
    setupNivel();
}

Nivel3::~Nivel3()
{
}

void Nivel3::setupNivel()
{
    tamanoVista = QSize(800, 600);
    setFixedSize(tamanoVista);

    nivelCompletado = false;
    juegoActivo = true;
    tiempoDesdeUltimoSpawn = 0;
    frameAnimacion = 0;
    tiempoAnimacion = 0;

    // PRECARGAR SPRITES ANTES de crear el jugador
    SpriteManager::getInstance().preloadGameSprites();

    jugadorN3 = new JugadorNivel3();
    jugador = jugadorN3;

    // Establecer posición inicial del jugador
    jugadorN3->setPosicion(QPointF(100, 400));

    posicionCamara = QPointF(0, 0);
    obstaculos.clear();
    powerUps.clear();

    // Cargar recursos de audio
    AudioManager::getInstance().loadSounds();

    if (timerJuego) {
        timerJuego->start(16);
    }

    timerNivel.start();
    qDebug() << "🎮 Nivel 3 iniciado";
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

    // Actualizar física del jugador
    jugadorN3->actualizar(deltaTime);

    // DEBUG: Mostrar posición del jugador ocasionalmente
    static int debugCounter = 0;
    if (debugCounter++ % 60 == 0) {
        qDebug() << "🎯 Jugador - Pos Y:" << jugadorN3->getPosicion().y()
                 << "Saltando:" << jugadorN3->estaSaltando
                 << "Agachado:" << jugadorN3->estaAgachado;
    }

    // Actualizar animación
    actualizarAnimacion(deltaTime);

    // Actualizar scroll
    actualizarCamaraAutoScroll();

    // Generar obstáculos
    tiempoDesdeUltimoSpawn += deltaTime;
    if (tiempoDesdeUltimoSpawn > spawnRate * 1000) {
        generarObstaculos();
        tiempoDesdeUltimoSpawn = 0;
    }

    // Verificar colisiones
    verificarColisiones();

    // Completar nivel por tiempo
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

    // Cambiar frame cada 100ms
    if (tiempoAnimacion > 100) {
        if (jugadorN3->estaSaltando || jugadorN3->estaAgachado) {
            // Estados especiales: frame fijo
            frameAnimacion = 0;
        } else {
            // Corriendo: 6 frames de animación
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

void Nivel3::generarObstaculos()
{
    QRandomGenerator* random = QRandomGenerator::global();

    float dificultad = qMin(distanciaRecorrida / 3000.0f, 1.0f);

    bool usarPatron = random->bounded(100) < (20 + (int)(dificultad * 30));

    if (usarPatron && distanciaRecorrida > 800) {
        int maxPatrones = dificultad > 0.7f ? 3 : 2;
        int patron = random->bounded(maxPatrones);
        generarPatronObstaculos(patron);
        qDebug() << "🎯 Patrón de obstáculos:" << patron;
    } else {
        generarObstaculosAleatorios();
    }

    qDebug() << "🎯 Generados" << obstaculos.size() << "obstáculos";
}

void Nivel3::generarPatronObstaculos(int tipoPatron)
{
    float baseX = posicionCamara.x() + width() + 200;

    switch (tipoPatron) {
    case 0: // Triple salto
        obstaculos.append(QRectF(baseX, 520, 50, 25));
        obstaculos.append(QRectF(baseX + 180, 520, 60, 25));
        obstaculos.append(QRectF(baseX + 360, 520, 70, 25));
        break;

    case 1: // Bajo-Alto-Bajo
        obstaculos.append(QRectF(baseX, 520, 60, 25));
        obstaculos.append(QRectF(baseX + 200, 450, 40, 80));
        obstaculos.append(QRectF(baseX + 400, 520, 70, 25));
        break;

    case 2: // Salto largo
        obstaculos.append(QRectF(baseX, 520, 150, 30));
        break;
    }
}

void Nivel3::generarObstaculosAleatorios()
{
    QRandomGenerator* random = QRandomGenerator::global();

    const int DISTANCIA_MINIMA = 250;
    const int DISTANCIA_MAXIMA = 600;

    int numObstaculos = random->bounded(1, 3);
    float ultimaPosicionX = posicionCamara.x() + width() + 150;

    for (int i = 0; i < numObstaculos; i++) {
        int separacion = random->bounded(DISTANCIA_MINIMA, DISTANCIA_MAXIMA);
        float x = ultimaPosicionX + separacion;

        int tipo = random->bounded(100);
        float ancho, alto, y;

        if (tipo < 50) {
            y = 530 - 25;
            ancho = random->bounded(40, 70);
            alto = 25;
        } else if (tipo < 80) {
            y = 530 - 60;
            ancho = random->bounded(50, 80);
            alto = 60;
        } else {
            y = 530 - 100;
            ancho = random->bounded(30, 50);
            alto = 100;
        }

        obstaculos.append(QRectF(x, y, ancho, alto));
        ultimaPosicionX = x + ancho + 50;
    }

    if (random->bounded(100) < 30) {
        float xPowerUp = ultimaPosicionX + 100;
        float yPowerUp = 450;
        powerUps.append(QRectF(xPowerUp, yPowerUp, 30, 30));
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
            qDebug() << "💥 COLISIÓN!";
            jugadorN3->setVida(jugadorN3->getVida() - 1);
            AudioManager::getInstance().playPlayerHurt();
            obstaculos.removeAt(i);

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
        }
        break;
    case Qt::Key_S:
        if (!jugadorN3->estaSaltando) {
            jugadorN3->agacharse();
            AudioManager::getInstance().playPlayerMove();
        }
        break;
    case Qt::Key_P:
        pausarNivel();
        break;
    case Qt::Key_R:
        reanudarNivel();
        break;
    }
}

void Nivel3::keyReleaseEvent(QKeyEvent *event)
{
    switch(event->key()) {
    case Qt::Key_S:
        jugadorN3->levantarse();
        break;
    }
}

void Nivel3::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Fondo
    painter.fillRect(rect(), QColor(100, 150, 255));

    // Suelo
    painter.fillRect(0, 550, width(), height() - 550, QColor(100, 200, 100));

    // Obstáculos
    painter.setBrush(QBrush(QColor(150, 75, 0)));
    painter.setPen(QPen(Qt::black, 2));
    for (const QRectF& obstaculo : obstaculos) {
        QRectF obstaculoVista = obstaculo.translated(-posicionCamara);
        if (obstaculoVista.right() > 0 && obstaculoVista.left() < width()) {
            painter.drawRect(obstaculoVista);
        }
    }

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

void Nivel3::dibujarJugador(QPainter &painter)
{
    float jugadorY = jugadorN3->getPosicion().y();

    QString spriteName;
    int frameIndex = frameAnimacion;
    int frameWidth, frameHeight;
    QSize displaySize(80, 100); // Tamaño de visualización en pantalla

    // Determinar qué sprite usar según el estado
    if (jugadorN3->estaSaltando) {
        spriteName = "player_move"; // Usar sprite de correr para salto
        frameWidth = 1152 / 6; // 192 pixels por frame
        frameHeight = 192;
        frameIndex = 0; // Primer frame para salto
    } else if (jugadorN3->estaAgachado) {
        spriteName = "player_idle";
        frameWidth = 1536 / 8; // 192 pixels por frame
        frameHeight = 192;
        displaySize = QSize(80, 70); // Más bajo cuando está agachado
        frameIndex = 0; // Frame fijo para agachado
    } else {
        spriteName = "player_move"; // Animación de correr
        frameWidth = 1152 / 6; // 192 pixels por frame
        frameHeight = 192;
        frameIndex = frameAnimacion % 6; // 6 frames de animación
    }

    QPixmap spriteSheet = SpriteManager::getInstance().getSprite(spriteName);

    if(!spriteSheet.isNull()) {
        // Calcular frame rect basado en los tamaños reales
        QRect frameRect(frameIndex * frameWidth, 0, frameWidth, frameHeight);
        QPixmap frame = spriteSheet.copy(frameRect);

        // Dibujar en posición fija en X (100) y Y variable
        QRectF displayRect(100 - displaySize.width()/2,
                           jugadorY - displaySize.height()/2,
                           displaySize.width(),
                           displaySize.height());
        painter.drawPixmap(displayRect, frame, frame.rect());

        // Debug ocasional
        static int debugCounter = 0;
        if (debugCounter++ % 120 == 0) {
            qDebug() << "🎨 Sprite:" << spriteName
                     << "Frame:" << frameIndex
                     << "Pos Y:" << jugadorY
                     << "Estado - Saltando:" << jugadorN3->estaSaltando
                     << "Agachado:" << jugadorN3->estaAgachado;
        }

    } else {
        // Fallback visual
        painter.setBrush(QBrush(QColor(0, 100, 200)));
        painter.setPen(QPen(Qt::white, 3));
        QRectF jugadorRect(80, jugadorY - 25, 40, 50);
        painter.drawRect(jugadorRect);
        qDebug() << "❌ Sprite no encontrado:" << spriteName;
    }
}

void Nivel3::dibujarHUD(QPainter &painter)
{
    float tiempoRestante = tiempoObjetivo - (timerNivel.elapsed() / 1000.0f);
    if (tiempoRestante < 0) tiempoRestante = 0;

    // Fondo del HUD con sprite - POSICIÓN ABSOLUTA
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

    // Barra de tiempo - CORREGIDO: Usar posición absoluta
    float progresoTiempo = tiempoRestante / tiempoObjetivo;
    QPixmap ribbon = UIManager::getInstance().getRibbonRed();
    if (!ribbon.isNull()) {
        // Escalar el ribbon al tamaño deseado
        QPixmap ribbonEscalado = ribbon.scaled(200, 30);

        // Calcular el ancho visible basado en el progreso
        int anchoVisible = static_cast<int>(200 * progresoTiempo);

        if (anchoVisible > 0) {
            // Crear una versión recortada
            QPixmap ribbonRecortado = ribbonEscalado.copy(0, 0, anchoVisible, 30);
            painter.drawPixmap(220, 15, ribbonRecortado);
        }
    } else {
        // Fallback: barra de progreso simple
        painter.setBrush(QBrush(QColor(255, 0, 0, 180)));
        painter.setPen(Qt::NoPen);
        painter.drawRect(220, 15, static_cast<int>(200 * progresoTiempo), 20);
    }

    // Controles en la parte inferior - POSICIÓN ABSOLUTA
    UIManager::getInstance().drawText(painter,
                                      "ESPACIO: Saltar   S: Agacharse",
                                      width()/2 - 150, height() - 30);
}
