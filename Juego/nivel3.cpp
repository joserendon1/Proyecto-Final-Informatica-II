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
    , jugadorN3(nullptr)
    , nivelCompletado(false)
    , juegoActivo(true)
    , tiempoDesdeUltimoSpawn(0)
{
    setupNivel();
}

Nivel3::~Nivel3()
{
    if (timerJuego) {
        timerJuego->stop();
    }
}

void Nivel3::setupNivel()
{
    // Configuración específica del nivel 3
    velocidadScroll = 2.0f;
    distanciaRecorrida = 0;
    tiempoObjetivo = 90.0f; // 1.5 minutos
    spawnRate = 2.0f; // obstáculos cada 2 segundos

    nivelCompletado = false;
    juegoActivo = true;
    tiempoDesdeUltimoSpawn = 0;

    // Crear jugador específico para nivel 3
    jugadorN3 = new JugadorNivel3();
    jugador = jugadorN3; // Asignar al puntero base

    // Configurar cámara inicial
    posicionCamara = QPointF(0, 0);

    // Generar obstáculos iniciales
    generarObstaculos();

    // Iniciar timer
    timerNivel.start();

    qDebug() << "🎮 Nivel 3 - Auto-scroller iniciado";
}

void Nivel3::iniciarNivel()
{
    NivelBase::iniciarNivel();
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

    // Actualizar scroll automático
    actualizarCamaraAutoScroll();

    // Actualizar jugador
    if (jugadorN3->estaSaltando) {
        jugadorN3->tiempoSalto += deltaTime / 1000.0f;
        if (jugadorN3->tiempoSalto > 0.5f) {
            jugadorN3->estaSaltando = false;
            jugadorN3->tiempoSalto = 0;
        }
    }

    // Generar obstáculos periódicamente
    tiempoDesdeUltimoSpawn += deltaTime;
    if (tiempoDesdeUltimoSpawn > spawnRate * 1000) {
        generarObstaculos();
        tiempoDesdeUltimoSpawn = 0;
    }

    // Verificar colisiones
    verificarColisiones();

    // Verificar si se completó el nivel
    if (timerNivel.elapsed() / 1000.0f >= tiempoObjetivo) {
        nivelCompletado = true;
        juegoActivo = false;
        emit levelCompleted();
        AudioManager::getInstance().playLevelUp();
    }
}

void Nivel3::actualizarCamaraAutoScroll()
{
    // Movimiento automático de la cámara hacia la derecha
    posicionCamara.setX(posicionCamara.x() + velocidadScroll);
    distanciaRecorrida += velocidadScroll;

    // El jugador se mantiene en una posición X fija (solo se mueve en Y)
    jugadorN3->setPosicionX(100); // Usar setter en lugar de acceso directo
}

void Nivel3::generarObstaculos()
{
    QRandomGenerator* random = QRandomGenerator::global();

    // Generar entre 1 y 3 obstáculos
    int numObstaculos = random->bounded(1, 4);

    for (int i = 0; i < numObstaculos; i++) {
        float x = posicionCamara.x() + tamanoVista.width() + random->bounded(100, 300);
        float y = random->bounded(200, 500);
        float ancho = random->bounded(30, 80);
        float alto = random->bounded(30, 60);

        obstaculos.append(QRectF(x, y, ancho, alto));
    }

    // Ocasionalmente generar power-ups
    if (random->bounded(100) < 20) {
        float x = posicionCamara.x() + tamanoVista.width() + random->bounded(200, 400);
        float y = random->bounded(200, 500);
        powerUps.append(QRectF(x, y, 25, 25));
    }
}

void Nivel3::verificarColisiones()
{
    QRectF areaJugador = jugadorN3->getAreaColision();

    // Verificar colisión con obstáculos
    for (int i = obstaculos.size() - 1; i >= 0; i--) {
        if (areaJugador.intersects(obstaculos[i])) {
            jugadorN3->setVida(jugadorN3->getVida() - 1); // Usar setter
            AudioManager::getInstance().playPlayerHurt();
            obstaculos.removeAt(i);

            if (jugadorN3->getVida() <= 0) {
                juegoActivo = false;
                emit gameOver();
            }
        }
    }

    // Verificar colisión con power-ups
    for (int i = powerUps.size() - 1; i >= 0; i--) {
        if (areaJugador.intersects(powerUps[i])) {
            float nuevaVida = std::min(jugadorN3->getVida() + 1.0f, 5.0f); // Usar std::min
            jugadorN3->setVida(nuevaVida);
            AudioManager::getInstance().playLevelUp();
            powerUps.removeAt(i);
        }
    }

    // Limpiar obstáculos que ya pasaron
    for (int i = obstaculos.size() - 1; i >= 0; i--) {
        if (obstaculos[i].right() < posicionCamara.x()) {
            obstaculos.removeAt(i);
        }
    }

    // Limpiar power-ups que ya pasaron
    for (int i = powerUps.size() - 1; i >= 0; i--) {
        if (powerUps[i].right() < posicionCamara.x()) {
            powerUps.removeAt(i);
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

    // Dibujar fondo
    painter.fillRect(rect(), QColor(100, 150, 255));

    // Dibujar suelo
    painter.fillRect(0, 550, width(), height() - 550, QColor(100, 200, 100));

    // Dibujar obstáculos
    painter.setBrush(QBrush(QColor(150, 75, 0)));
    painter.setPen(Qt::NoPen);
    for (const QRectF& obstaculo : obstaculos) {
        QRectF obstaculoVista = obstaculo.translated(-posicionCamara);
        if (estaEnVista(obstaculoVista)) {
            painter.drawRect(obstaculoVista);
        }
    }

    // Dibujar power-ups
    painter.setBrush(QBrush(QColor(255, 255, 0)));
    for (const QRectF& powerUp : powerUps) {
        QRectF powerUpVista = powerUp.translated(-posicionCamara);
        if (estaEnVista(powerUpVista)) {
            painter.drawEllipse(powerUpVista);
        }
    }

    // Dibujar jugador
    painter.setBrush(QBrush(QColor(0, 100, 200)));
    QRectF jugadorRect = jugadorN3->getAreaColision().translated(-posicionCamara);
    painter.drawRect(jugadorRect);

    // Dibujar HUD
    dibujarHUD(painter);

    // Mensaje de nivel completado
    if (nivelCompletado) {
        painter.fillRect(rect(), QColor(0, 0, 0, 150));
        UIManager::getInstance().drawText(painter, "¡NIVEL COMPLETADO!",
                                          width()/2 - 150, height()/2, 2.0f);
    }
}

void Nivel3::dibujarHUD(QPainter &painter)
{
    float tiempoRestante = tiempoObjetivo - (timerNivel.elapsed() / 1000.0f);
    if (tiempoRestante < 0) tiempoRestante = 0;

    // Panel de información
    painter.setBrush(QBrush(QColor(0, 0, 0, 150)));
    painter.setPen(Qt::NoPen);
    painter.drawRect(10, 10, 200, 80);

    // Textos del HUD
    UIManager::getInstance().drawText(painter,
                                      QString("Vidas: %1").arg((int)jugadorN3->getVida()), 20, 30);
    UIManager::getInstance().drawText(painter,
                                      QString("Tiempo: %1s").arg((int)tiempoRestante), 20, 50);
    UIManager::getInstance().drawText(painter,
                                      QString("Distancia: %1m").arg((int)(distanciaRecorrida / 10)), 20, 70);

    // Instrucciones
    UIManager::getInstance().drawText(painter,
                                      "ESPACIO: Saltar   S: Agacharse", width() - 300, height() - 20);
}
