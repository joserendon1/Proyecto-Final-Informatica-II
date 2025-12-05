#include "nivel2.h"
#include "audiomanager.h"
#include "spritemanager.h"
#include "uimanager.h"
#include <QPainter>
#include <QKeyEvent>
#include <QDebug>
#include <QRandomGenerator>
#include <QShowEvent>
#include <QDateTime>

Nivel2::Nivel2(QWidget *parent) : QWidget(parent)
    , frameAnimacion(0)
    , tiempoAnimacion(0)
    , tiempoTranscurrido(0)
    , tiempoObjetivo(90)
    , barrilesEsquivados(0)
    , puntuacion(0)
    , enPausa(false)
    , juegoIniciado(false)
    , nivelCompletado(false)
    , spawnRate(1.0f)
    , tiempoDesdeUltimoSpawn(0)
    , cooldownSonidoMovimiento(0)
    , jugadorSeEstaMoviendo(false)
{
    setFixedSize(ANCHO_VENTANA, ALTO_VENTANA);
    setFocusPolicy(Qt::StrongFocus);

    setupNivel();
}

Nivel2::~Nivel2()
{
    delete jugador;
    barriles.clear();
    AudioManager::getInstance().stopAllSounds();
}

void Nivel2::setupNivel()
{
    // Cargar recursos como en Nivel 1
    SpriteManager::getInstance().preloadGameSprites();
    UIManager::getInstance().loadResources();
    AudioManager::getInstance().loadSounds();

    jugador = new JugadorNivel2();

    timerJuego = new QTimer(this);
    timerJuego->setInterval(16); // ~60 FPS

    timerGeneracionBarriles = new QTimer(this);
    timerGeneracionBarriles->setInterval(1000); // Generar barril cada segundo

    connect(timerJuego, &QTimer::timeout, this, &Nivel2::actualizarJuego);
    connect(timerGeneracionBarriles, &QTimer::timeout, this, &Nivel2::generarBarril);

    // Verificar que los sprites se cargaron
    QPixmap testSprite = SpriteManager::getInstance().getSprite("obstacle3");
    qDebug() << "🎯 Sprite obstacle3 cargado:" << !testSprite.isNull() << "Tamaño:" << testSprite.size();

    if (testSprite.isNull()) {
        qDebug() << "⚠️ WARNING: obstacle3 no encontrado, usarán fallbacks";
    }
}

void Nivel2::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!juegoIniciado) {
        iniciarNivel();
        juegoIniciado = true;
    }
}

void Nivel2::iniciarNivel()
{
    barriles.clear();
    tiempoTranscurrido = 0;
    barrilesEsquivados = 0;
    puntuacion = 0;
    frameAnimacion = 0;
    tiempoAnimacion = 0;
    cooldownSonidoMovimiento = 0;
    jugadorSeEstaMoviendo = false;

    // Resetear jugador
    jugador->resetear();

    timerJuego->start();
    timerGeneracionBarriles->start();

    enPausa = false;
    nivelCompletado = false;
    juegoIniciado = true;

    timerNivel.start();

    // Reproducir música de fondo como en Nivel 1
    AudioManager::getInstance().stopBackgroundMusic(); // Asegurarse que no haya música previa
    AudioManager::getInstance().playBackgroundMusic();

    qDebug() << "🎮 Nivel 2 iniciado - Modo Esquiva de Barriles";

    update();
}

void Nivel2::pausarNivel()
{
    timerJuego->stop();
    timerGeneracionBarriles->stop();
    enPausa = true;

    // Pausar audio como en Nivel 1
    AudioManager::getInstance().stopAllSounds();

    emit gamePaused();
    update();
}

void Nivel2::reanudarNivel()
{
    timerJuego->start();
    timerGeneracionBarriles->start();
    enPausa = false;

    // Reanudar audio como en Nivel 1
    AudioManager::getInstance().playBackgroundMusic();

    emit gameResumed();
    update();
}

void Nivel2::actualizarJuego()
{
    if (enPausa || !juegoIniciado) return;

    // Calcular deltaTime similar al Nivel 1
    static qint64 tiempoAnterior = 0;
    qint64 tiempoActual = timerNivel.elapsed();
    float deltaTime = tiempoAnterior > 0 ? (tiempoActual - tiempoAnterior) / 1000.0f : 0.016f;
    tiempoAnterior = tiempoActual;

    // Actualizar tiempo
    tiempoTranscurrido = tiempoActual / 1000;

    // Verificar victoria
    if (tiempoTranscurrido >= tiempoObjetivo) {
        nivelCompletado = true;
        juegoIniciado = false;
        timerJuego->stop();
        timerGeneracionBarriles->stop();

        // Sonido de victoria como en Nivel 1
        AudioManager::getInstance().playLevelUp();
        emit levelCompleted();

        update();
        return;
    }

    // Actualizar animación del jugador
    actualizarAnimacion(deltaTime);

    // Actualizar posición del jugador
    jugador->actualizar(deltaTime);

    // Manejo de sonido de movimiento MEJORADO (como en Nivel 1)
    bool moviendoseAhora = jugador->estaMoviendose();

    if (moviendoseAhora && !jugadorSeEstaMoviendo) {
        // Comenzó a moverse - reproducir sonido
        AudioManager::getInstance().playPlayerMove();
        cooldownSonidoMovimiento = 30; // Cooldown en frames
        jugadorSeEstaMoviendo = true;
    } else if (moviendoseAhora && cooldownSonidoMovimiento <= 0) {
        // Sigue moviéndose y cooldown terminó
        AudioManager::getInstance().playPlayerMove();
        cooldownSonidoMovimiento = 30;
    } else if (!moviendoseAhora) {
        // Dejó de moverse
        jugadorSeEstaMoviendo = false;
    }

    if (cooldownSonidoMovimiento > 0) {
        cooldownSonidoMovimiento--;
    }

    // Actualizar posición de los barriles
    for (Barril& barril : barriles) {
        if (barril.activo) {
            barril.posicion.setY(barril.posicion.y() + barril.velocidad);
        }
    }

    procesarColisiones();
    limpiarBarriles();

    // Aumentar dificultad progresivamente
    if (tiempoTranscurrido > 30) { // Después de 30 segundos
        timerGeneracionBarriles->setInterval(800);
        spawnRate = 1.25f;
    }
    if (tiempoTranscurrido > 60) { // Después de 60 segundos
        timerGeneracionBarriles->setInterval(600);
        spawnRate = 1.5f;
    }
    if (tiempoTranscurrido > 75) { // Después de 75 segundos
        timerGeneracionBarriles->setInterval(400);
        spawnRate = 2.0f;
    }

    // Verificar derrota
    if (jugador->getVida() <= 0) {
        juegoIniciado = false;
        timerJuego->stop();
        timerGeneracionBarriles->stop();

        // Sonido de game over
        AudioManager::getInstance().playPlayerHurt();
        emit gameOver();

        update();
        return;
    }

    update();
}

void Nivel2::actualizarAnimacion(float deltaTime)
{
    tiempoAnimacion += deltaTime;

    if (tiempoAnimacion > 0.1f) { // 10 FPS para la animación
        frameAnimacion = (frameAnimacion + 1) % 6; // 6 frames de animación
        tiempoAnimacion = 0;
    }
}

void Nivel2::generarBarril()
{
    if (enPausa || !juegoIniciado) return;

    QRandomGenerator* random = QRandomGenerator::global();

    // Posición aleatoria en X (evitando los bordes)
    int posX = random->bounded(LIMITE_IZQUIERDO + 30, LIMITE_DERECHO - 30);
    QPointF posicion(posX, -50);

    // Velocidad aleatoria usando generateDouble() como en Nivel 3
    float velocidadBase = 3.0f + (tiempoTranscurrido / 30.0f);
    double randomFactor = random->generateDouble() * 0.4 + 0.8; // Escala a [0.8, 1.2]
    float velocidad = velocidadBase * static_cast<float>(randomFactor);

    // Tipo de barril (obstacle3 por defecto, otros según dificultad)
    int tipoBarril = 1; // obstacle3 por defecto
    int probabilidad = random->bounded(100);

    if (tiempoTranscurrido > 45 && probabilidad < 20) {
        tipoBarril = 2; // obstacle1 (grande) - 20% probabilidad después de 45s
    } else if (tiempoTranscurrido > 60 && probabilidad < 15) {
        tipoBarril = 3; // obstacle4 (pequeño) - 15% probabilidad después de 60s
    }

    barriles.append(Barril(posicion, velocidad, tipoBarril));

    // Sonido ocasional de barril cayendo (más suave que el movimiento del jugador)
    if (random->bounded(100) < 10) { // 10% de probabilidad
        AudioManager::getInstance().playEnemyHit(); // Usar sonido de golpe suave
    }
}

void Nivel2::procesarColisiones()
{
    for (Barril& barril : barriles) {
        if (barril.activo && barril.getAreaColision().intersects(jugador->getAreaColision())) {
            // Colisión detectada
            float danio = 0;
            switch(barril.tipo) {
            case 1: danio = 25.0f; break; // obstacle3 (normal)
            case 2: danio = 35.0f; break; // obstacle1 (grande)
            case 3: danio = 15.0f; break; // obstacle4 (pequeño)
            }

            jugador->recibirDanio(danio);
            barril.activo = false;

            // Sonido de daño
            AudioManager::getInstance().playPlayerHurt();

            qDebug() << "💥 Colisión con barril tipo" << barril.tipo
                     << "Vida restante:" << jugador->getVida();
        }
    }
}

void Nivel2::limpiarBarriles()
{
    for (int i = barriles.size() - 1; i >= 0; --i) {
        if (!barriles[i].activo || barriles[i].estaFueraDePantalla()) {
            if (barriles[i].estaFueraDePantalla() && barriles[i].activo) {
                barrilesEsquivados++;
                puntuacion += 10 * (4 - barriles[i].tipo); // Más puntos por barriles más difíciles
            }
            barriles.removeAt(i);
        }
    }
}

void Nivel2::keyPressEvent(QKeyEvent *event)
{
    if (!juegoIniciado) return;

    std::vector<bool> teclas(4, false);

    switch(event->key()) {
    case Qt::Key_A:
    case Qt::Key_Left:
        teclas[0] = true;
        break;
    case Qt::Key_D:
    case Qt::Key_Right:
        teclas[1] = true;
        break;
    case Qt::Key_P:
    case Qt::Key_Escape:
        pausarNivel();
        qDebug() << "⏸️ Nivel 2 pausado";
        break;
    case Qt::Key_R:
        reanudarNivel();
        qDebug() << "▶️ Nivel 2 reanudado";
        break;
    default:
        QWidget::keyPressEvent(event);
        return;
    }

    jugador->procesarInput(teclas);
    update();
}

void Nivel2::keyReleaseEvent(QKeyEvent *event)
{
    if (!juegoIniciado) return;

    std::vector<bool> teclas(4, false);

    switch(event->key()) {
    case Qt::Key_A:
    case Qt::Key_Left:
        teclas[0] = false;
        break;
    case Qt::Key_D:
    case Qt::Key_Right:
        teclas[1] = false;
        break;
    default:
        QWidget::keyReleaseEvent(event);
        return;
    }

    jugador->procesarInput(teclas);
    update();
}

void Nivel2::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Dibujar fondo
    dibujarFondo(painter);

    // Dibujar suelo
    dibujarSuelo(painter);

    // Dibujar barriles
    dibujarBarriles(painter);

    // Dibujar jugador
    dibujarJugador(painter);

    // Dibujar HUD
    dibujarHUD(painter);

    // Mostrar mensaje de pausa (similar al Nivel 1)
    if (enPausa) {
        painter.fillRect(rect(), QColor(0, 0, 0, 150));
        UIManager::getInstance().drawText(painter, "PAUSA",
                                          width()/2 - 100, height()/2, 2.0f);
    }

    // Mostrar mensaje de victoria (similar al Nivel 1)
    if (nivelCompletado) {
        painter.fillRect(rect(), QColor(0, 0, 0, 150));

        UIManager::getInstance().drawText(painter, "¡NIVEL COMPLETADO!",
                                          width()/2 - 150, height()/2 - 50, 2.0f);
        UIManager::getInstance().drawText(painter,
                                          QString("Puntuación: %1").arg(puntuacion),
                                          width()/2 - 100, height()/2, 1.5f);
        UIManager::getInstance().drawText(painter,
                                          QString("Barriles esquivados: %1").arg(barrilesEsquivados),
                                          width()/2 - 120, height()/2 + 50, 1.2f);
    }
}

// ========== MÉTODOS HELPER SIMILARES AL NIVEL 1 ==========

void Nivel2::dibujarEntidadConSprite(QPainter &painter, const QPointF &posicion,
                                     const QString &spriteName, const QSize &displaySize,
                                     int frameWidth, int frameHeight, int currentFrame)
{
    // EXACTAMENTE IGUAL AL Nivel 1
    QPixmap spriteSheet = SpriteManager::getInstance().getSprite(spriteName);

    if(!spriteSheet.isNull()) {
        QRect frameRect(currentFrame * frameWidth, 0, frameWidth, frameHeight);
        QPixmap frame = spriteSheet.copy(frameRect);

        QRectF displayRect(posicion.x() - displaySize.width()/2,
                           posicion.y() - displaySize.height()/2,
                           displaySize.width(), displaySize.height());
        painter.drawPixmap(displayRect, frame, frame.rect());
    } else {
        // Fallback simple
        dibujarEntidadSimple(painter, posicion, displaySize, QColor(255, 100, 100));
    }
}

void Nivel2::dibujarEntidadSimple(QPainter &painter, const QPointF &posicion,
                                  const QSize &displaySize, const QColor &color)
{
    painter.setBrush(QBrush(color));
    painter.setPen(QPen(Qt::white, 2));
    painter.drawEllipse(posicion, displaySize.width()/2, displaySize.height()/2);
}

void Nivel2::dibujarFondo(QPainter &painter)
{
    // Intentar usar sprite de fondo como en Nivel 1
    QPixmap fondo = SpriteManager::getInstance().getSprite("background2");

    if (!fondo.isNull()) {
        // Dibujar fondo escalado
        painter.drawPixmap(rect(), fondo);
    } else {
        // Fallback: gradiente azul cielo (similar al Nivel 1)
        QLinearGradient gradient(0, 0, 0, height());
        gradient.setColorAt(0, QColor(135, 206, 235));
        gradient.setColorAt(1, QColor(100, 180, 220));
        painter.fillRect(rect(), gradient);
    }
}

void Nivel2::dibujarSuelo(QPainter &painter)
{
    // Intentar usar sprite de suelo como en Nivel 1
    QPixmap suelo = SpriteManager::getInstance().getSprite("ground2");

    if (!suelo.isNull()) {
        // Dibujar suelo con patrón repetido
        int sueloY = SUELO_Y;
        for (int x = 0; x < width(); x += suelo.width()) {
            painter.drawPixmap(x, sueloY, suelo.width(), height() - sueloY, suelo);
        }
    } else {
        // Fallback: suelo verde sólido
        painter.setBrush(QColor(34, 139, 34));
        painter.setPen(Qt::NoPen);
        painter.drawRect(0, SUELO_Y, width(), height() - SUELO_Y);
    }
}

void Nivel2::dibujarBarriles(QPainter &painter)
{
    for (const Barril& barril : qAsConst(barriles)) {
        if (barril.activo) {
            QPointF pos = barril.posicion;
            QSize displaySize = barril.getDisplaySize();

            QString spriteName = barril.getSpriteName();

            // Intentar usar sprite
            QPixmap barrilSprite = SpriteManager::getInstance().getSprite(spriteName);

            if (!barrilSprite.isNull()) {
                // Usar sprite sin animación (frame 0)
                dibujarEntidadConSprite(painter, pos, spriteName, displaySize,
                                        barrilSprite.width(), barrilSprite.height(), 0);
            } else {
                // Fallback con colores
                QColor color;
                switch(barril.tipo) {
                case 1: color = QColor(139, 69, 19); break;  // Marrón obstacle3
                case 2: color = QColor(101, 67, 33); break;  // Marrón oscuro obstacle1
                case 3: color = QColor(160, 120, 80); break; // Marrón claro obstacle4
                }
                dibujarEntidadSimple(painter, pos, displaySize, color);
            }
        }
    }
}

void Nivel2::dibujarJugador(QPainter &painter)
{
    QPointF pos = jugador->getPosicion();

    // Determinar qué sprite usar (igual que Nivel 1)
    QString spriteName = jugador->estaMoviendose() ? "player_move" : "player_idle";
    QSize displaySize(60, 80);

    // Configuración de frames igual que Nivel 1
    int frameWidth = 192;
    int frameHeight = 192;
    int totalFrames = spriteName == "player_move" ? 6 : 8;

    // Calcular frame actual
    int currentFrame = frameAnimacion % totalFrames;

    // Usar el mismo método que el Nivel 1
    dibujarEntidadConSprite(painter, pos, spriteName, displaySize, frameWidth, frameHeight, currentFrame);

// Dibujar área de colisión en modo debug
#ifdef QT_DEBUG
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(Qt::red, 1, Qt::DotLine));
    QRectF colisionRect = jugador->getAreaColision();
    painter.drawRect(colisionRect);
#endif
}

void Nivel2::dibujarHUD(QPainter &painter)
{
    UIManager& ui = UIManager::getInstance();

    // HUD similar al Nivel 1
    const int hudX = 10;
    const int hudY = 10;
    const int panelWidth = 200;
    const int panelHeight = 120;

    // Panel de fondo
    if(!ui.getHudPanel().isNull()) {
        painter.drawPixmap(hudX, hudY, panelWidth, panelHeight, ui.getHudPanel());
    } else {
        painter.fillRect(hudX, hudY, panelWidth, panelHeight, QColor(0, 0, 0, 180));
    }

    int panelCenterX = hudX + panelWidth / 2;
    int textY = hudY + 30;
    float scale = 1.0f;
    int lineHeight = 25;

    // Tiempo restante
    int tiempoRestante = tiempoObjetivo - tiempoTranscurrido;
    if (tiempoRestante < 0) tiempoRestante = 0;

    QString tiempoText = QString("TIEMPO: %1/%2").arg(tiempoRestante).arg(tiempoObjetivo);
    int tiempoWidth = ui.getTextWidth(tiempoText, scale);
    ui.drawText(painter, tiempoText, panelCenterX - tiempoWidth/2, textY, scale);
    textY += lineHeight;

    // Vida con barra visual (como en Nivel 1)
    QString vidaText = QString("VIDA: %1").arg((int)jugador->getVida());
    int vidaWidth = ui.getTextWidth(vidaText, scale);
    ui.drawText(painter, vidaText, panelCenterX - vidaWidth/2, textY, scale);
    textY += lineHeight;

    // Barra de vida
    float vidaPorcentaje = jugador->getVida() / 100.0f;
    painter.setBrush(QColor(255, 0, 0, 180));
    painter.setPen(Qt::NoPen);
    painter.drawRect(hudX + 10, textY - 10, 180, 12);

    painter.setBrush(QColor(0, 255, 0, 200));
    painter.drawRect(hudX + 10, textY - 10, 180 * vidaPorcentaje, 12);
    // Barra de progreso de tiempo (similar al Nivel 1)
    float progresoTiempo = tiempoTranscurrido / (float)tiempoObjetivo;
    painter.setBrush(QColor(255, 215, 0, 180));
    painter.setPen(Qt::NoPen);
    painter.drawRect(width()/2 - 100, 15, 200 * progresoTiempo, 10);

    // Borde de la barra
    painter.setPen(QPen(Qt::white, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(width()/2 - 100, 15, 200, 10);

    // Controles (lado derecho, similar al Nivel 1)
    int controlesX = width() - 220;
    int controlesY = 30;

    ui.drawText(painter, "CONTROLES:", controlesX, controlesY, 1.0f);
    ui.drawText(painter, "A/← - Izquierda", controlesX, controlesY + 20, 0.9f);
    ui.drawText(painter, "D/→ - Derecha", controlesX, controlesY + 40, 0.9f);
    ui.drawText(painter, "P/ESC - Pausar", controlesX, controlesY + 60, 0.9f);
    ui.drawText(painter, "R - Reanudar", controlesX, controlesY + 80, 0.9f);
}
