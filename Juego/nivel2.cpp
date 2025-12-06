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
#include <math.h>

struct Nivel2::Barril {
    QPointF posicion;
    float velocidad;
    bool activo;
    int tipoMovimiento; // 1: Caída lineal, 2: Oscilatorio, 3: Parabólico, 4: Caída con rebote
    float tiempoVida;   // Para animaciones
    float amplitud;     // Para movimiento oscilatorio
    float frecuencia;   // Para movimiento oscilatorio
    float fase;         // Para movimiento oscilatorio/parabólico
    float velocidadHorizontal; // Para movimiento parabólico
    float gravedad;     // Para movimiento parabólico

    Barril(QPointF pos, float vel, int tipo = 1) :
        posicion(pos), velocidad(vel), activo(true),
        tipoMovimiento(tipo), tiempoVida(0.0f),
        amplitud(30.0f), frecuencia(2.0f), fase(0.0f),
        velocidadHorizontal(0.0f), gravedad(0.5f)
    {
        // Configurar según tipo de movimiento
        QRandomGenerator* random = QRandomGenerator::global();

        if (tipoMovimiento == 2) { // Oscilatorio
            // Usar generateDouble() para valores float
            amplitud = 20.0f + static_cast<float>(random->generateDouble() * 30.0f); // [20, 50]
            frecuencia = 1.0f + static_cast<float>(random->generateDouble() * 2.0f); // [1, 3]
            fase = static_cast<float>(random->generateDouble() * 6.28f); // [0, 2π]
        }
        else if (tipoMovimiento == 3) { // Parabólico
            // Usar bounded() con valores int para el rango [-3, 3]
            velocidadHorizontal = -3.0f + random->bounded(7); // -3 + [0,6] = [-3,3]
            gravedad = 0.3f + static_cast<float>(random->generateDouble() * 0.4f); // [0.3, 0.7]
        }
    }

    void actualizar(float deltaTime) {
        tiempoVida += deltaTime;

        switch(tipoMovimiento) {
        case 1: // Caída lineal (actual)
            posicion.setY(posicion.y() + velocidad);
            break;

        case 2: // Movimiento oscilatorio (senoidal)
            posicion.setY(posicion.y() + velocidad);
            posicion.setX(posicion.x() + amplitud * qSin(frecuencia * tiempoVida + fase) * 0.1f);
            break;

        case 3: // Movimiento parabólico
            posicion.setY(posicion.y() + velocidad + gravedad * tiempoVida);
            posicion.setX(posicion.x() + velocidadHorizontal);
            break;

        case 4: // Caída con rebotes simulados
            posicion.setY(posicion.y() + velocidad);
            // Simular pequeños rebotes horizontales aleatorios
            QRandomGenerator* random = QRandomGenerator::global();
            if (random->bounded(100) < 20) { // 20% de probabilidad por frame
                // Usar generateDouble() para valores float pequeños
                float rebote = -2.0f + static_cast<float>(random->generateDouble() * 4.0f); // [-2, 2]
                posicion.setX(posicion.x() + rebote);
            }
            break;
        }
    }

    QRectF getAreaColision() const {
        // Área de colisión basada en el movimiento
        float factor = 1.0f;
        if (tipoMovimiento == 3) factor = 0.9f; // Parabólico - un poco más pequeño
        return QRectF(posicion.x() - 15 * factor,
                      posicion.y() - 15 * factor,
                      30 * factor, 30 * factor);
    }

    QString getSpriteName() const {
        // Usamos obstacle3 para todos
        return "obstacle3";
    }

    QColor getColor() const {
        // Colores diferentes para cada tipo de movimiento
        switch(tipoMovimiento) {
        case 1: return QColor(139, 69, 19);   // Marrón - Lineal
        case 2: return QColor(160, 82, 45);   // Sienna - Oscilatorio
        case 3: return QColor(101, 67, 33);   // Marrón oscuro - Parabólico
        case 4: return QColor(160, 120, 80);  // Marrón claro - Con rebote
        default: return QColor(139, 69, 19);
        }
    }

    QSize getDisplaySize() const {
        // Tamaño ligeramente diferente según tipo
        switch(tipoMovimiento) {
        case 1: return QSize(30, 30);
        case 2: return QSize(32, 28);  // Algo achatado
        case 3: return QSize(28, 32);  // Algo estirado
        case 4: return QSize(30, 30);
        default: return QSize(30, 30);
        }
    }

    bool estaFueraDePantalla() const {
        return posicion.y() > 768 || posicion.x() < -50 || posicion.x() > 1074;
    }
};

Nivel2::Nivel2(QWidget *parent) : QWidget(parent)
    , timerJuego(nullptr)
    , timerGeneracionBarriles(nullptr)
    , jugador(nullptr)
    , frameAnimacion(0)
    , tiempoAnimacion(0)
    , tiempoTranscurrido(0)
    , tiempoObjetivo(60)
    , barrilesEsquivados(0)
    , puntuacion(0)
    , enPausa(false)
    , juegoIniciado(false)
    , nivelCompletado(false)
    , spawnRate(1.0f)
    , tiempoDesdeUltimoSpawn(0)
    , cooldownSonidoMovimiento(0)
    , jugadorSeEstaMoviendo(false)
    , frameAnimacionIdle(0)
    , frameAnimacionMove(0)
    , tiempoAnimacionIdle(0)
    , tiempoAnimacionMove(0)

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
    // Cargar recursos
    SpriteManager::getInstance().preloadGameSprites();
    UIManager::getInstance().loadResources();
    AudioManager::getInstance().loadSounds();

    jugador = new JugadorNivel2();

    timerJuego = new QTimer(this);
    timerJuego->setInterval(16); // ~60 FPS

    timerGeneracionBarriles = new QTimer(this);
    timerGeneracionBarriles->setInterval(1000); // Generar barril cada segundo inicialmente

    connect(timerJuego, &QTimer::timeout, this, &Nivel2::actualizarJuego);
    connect(timerGeneracionBarriles, &QTimer::timeout, this, &Nivel2::generarBarril);

    QPixmap testSprite = SpriteManager::getInstance().getSprite("obstacle3");

    if (testSprite.isNull()) {
        qDebug() << "obstacle3 no encontrado, usarán fallbacks";
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

    QRandomGenerator* random = QRandomGenerator::global();

    // Inicializar frames de animación
    frameAnimacionIdle = random->bounded(8);  // Frame aleatorio entre 0-7
    frameAnimacionMove = random->bounded(6);  // Frame aleatorio entre 0-5

    // Inicializar tiempos con valores aleatorios para que las animaciones comiencen inmediatamente
    tiempoAnimacionIdle = static_cast<float>(random->generateDouble() * 0.125f);  // Valor aleatorio entre 0-0.125s
    tiempoAnimacionMove = static_cast<float>(random->generateDouble() * 0.1f);    // Valor aleatorio entre 0-0.1s

    // Resetear jugador
    jugador->resetear();

    timerJuego->start();
    timerGeneracionBarriles->start();

    enPausa = false;
    nivelCompletado = false;
    juegoIniciado = true;

    timerNivel.start();

    // Reproducir música de fondo
    AudioManager::getInstance().stopBackgroundMusic();
    AudioManager::getInstance().playBackgroundMusic();

    qDebug() << "Nivel 2 iniciado - Modo Esquiva de Barriles";
    qDebug() << "Animación inicial - Idle frame:" << frameAnimacionIdle
             << "Move frame:" << frameAnimacionMove;

    update();
}

void Nivel2::pausarNivel()
{
    timerJuego->stop();
    timerGeneracionBarriles->stop();
    enPausa = true;

    AudioManager::getInstance().stopAllSounds();

    emit gamePaused();
    update();
}

void Nivel2::reanudarNivel()
{
    timerJuego->start();
    timerGeneracionBarriles->start();
    enPausa = false;

    AudioManager::getInstance().playBackgroundMusic();

    emit gameResumed();
    update();
}

void Nivel2::actualizarJuego()
{
    if (enPausa || !juegoIniciado) return;

    static qint64 tiempoAnterior = 0;
    static bool primerFrame = true;  // Variable para detectar el primer frame
    qint64 tiempoActual = timerNivel.elapsed();
    float deltaTime = tiempoAnterior > 0 ? (tiempoActual - tiempoAnterior) / 1000.0f : 0.016f;
    tiempoAnterior = tiempoActual;

    tiempoTranscurrido = tiempoActual / 1000;

    if (tiempoTranscurrido >= tiempoObjetivo) {
        nivelCompletado = true;
        juegoIniciado = false;
        timerJuego->stop();
        timerGeneracionBarriles->stop();

        AudioManager::getInstance().playLevelUp();
        emit levelCompleted();

        update();
        return;
    }

    actualizarAnimacion(deltaTime);

    // Si es el primer frame, forzar un frame de animación inmediato
    if (primerFrame) {
        primerFrame = false;

        // Forzar al menos un cambio de frame para que la animación sea visible inmediatamente
        QRandomGenerator* random = QRandomGenerator::global();
        frameAnimacionIdle = random->bounded(8);
        frameAnimacionMove = random->bounded(6);

        // También forzar la actualización visual
        update();
    }

    jugador->actualizar(deltaTime);

    bool moviendoseAhora = jugador->estaMoviendose();

    if (moviendoseAhora && !jugadorSeEstaMoviendo) {
        AudioManager::getInstance().playPlayerMove();
        cooldownSonidoMovimiento = 30;
        jugadorSeEstaMoviendo = true;
    } else if (moviendoseAhora && cooldownSonidoMovimiento <= 0) {
        AudioManager::getInstance().playPlayerMove();
        cooldownSonidoMovimiento = 30;
    } else if (!moviendoseAhora) {
        jugadorSeEstaMoviendo = false;
    }

    if (cooldownSonidoMovimiento > 0) {
        cooldownSonidoMovimiento--;
    }

    // Actualizar posición de los barriles usando física
    for (Barril& barril : barriles) {
        if (barril.activo) {
            barril.actualizar(deltaTime);
        }
    }

    procesarColisiones();
    limpiarBarriles();

    // Aumentar dificultad - usar tiempo acumulado en lugar de cambiar timer
    tiempoDesdeUltimoSpawn += deltaTime;

    float tiempoEntreSpawn = 1.0f;
    if (tiempoTranscurrido > 30) {
        tiempoEntreSpawn = 0.8f;
    }
    if (tiempoTranscurrido > 60) {
        tiempoEntreSpawn = 0.6f;
    }
    if (tiempoTranscurrido > 75) {
        tiempoEntreSpawn = 0.4f;
    }

    // Generar barriles según el tiempo acumulado
    if (tiempoDesdeUltimoSpawn >= tiempoEntreSpawn) {
        generarBarril();
        tiempoDesdeUltimoSpawn = 0;
    }

    // Verificar derrota
    if (jugador->getVida() <= 0) {
        juegoIniciado = false;
        timerJuego->stop();
        timerGeneracionBarriles->stop();

        AudioManager::getInstance().playPlayerHurt();
        emit gameOver();

        update();
        return;
    }

    update();
}

void Nivel2::actualizarAnimacion(float deltaTime)
{
    tiempoAnimacionIdle += deltaTime;
    tiempoAnimacionMove += deltaTime;

    // CORREGIR: Usar valores de tiempo lógicos (0.1 segundos, no 100)
    // Animación idle (8 frames, 8 FPS = 0.125 segundos por frame)
    if (tiempoAnimacionIdle > 0.125f) {
        frameAnimacionIdle = (frameAnimacionIdle + 1) % 8;
        tiempoAnimacionIdle = 0;
    }

    // Animación move (6 frames, 10 FPS = 0.1 segundos por frame)
    if (tiempoAnimacionMove > 0.1f) {
        frameAnimacionMove = (frameAnimacionMove + 1) % 6;
        tiempoAnimacionMove = 0;
    }
}

void Nivel2::generarBarril()
{
    if (enPausa || !juegoIniciado) return;

    QRandomGenerator* random = QRandomGenerator::global();

    // Posición aleatoria en X
    int posX = random->bounded(LIMITE_IZQUIERDO + 30, LIMITE_DERECHO - 30);
    QPointF posicion(posX, -50);

    // Velocidad base según dificultad
    float velocidadBase = 3.0f + (tiempoTranscurrido / 45.0f);
    velocidadBase = qMin(velocidadBase, 8.0f);

    // Usar generateDouble() para valores float
    double randomFactor = random->generateDouble() * 0.5 + 0.75; // [0.75, 1.25]
    float velocidad = velocidadBase * static_cast<float>(randomFactor);

    // Tipo de movimiento aleatorio
    int tipoMovimiento = 1;
    int probabilidad = random->bounded(100);

    if (tiempoTranscurrido < 20) {
        if (probabilidad < 80) tipoMovimiento = 1;
        else tipoMovimiento = 2;
    }
    else if (tiempoTranscurrido < 50) {
        if (probabilidad < 60) tipoMovimiento = 1;
        else if (probabilidad < 85) tipoMovimiento = 2;
        else tipoMovimiento = 3;
    }
    else {
        if (probabilidad < 40) tipoMovimiento = 1;
        else if (probabilidad < 70) tipoMovimiento = 2;
        else if (probabilidad < 90) tipoMovimiento = 3;
        else tipoMovimiento = 4;
    }

    barriles.append(Barril(posicion, velocidad, tipoMovimiento));
}

void Nivel2::procesarColisiones()
{
    bool colisionDetectada = false;

    for (Barril& barril : barriles) {
        if (barril.activo) {
            QRectF areaBarril = barril.getAreaColision();
            QRectF areaJugador = jugador->getAreaColision();

            // Verificar colisión con margen de tolerancia
            if (areaBarril.intersects(areaJugador)) {
                // Verificar solapamiento real (no solo bounding boxes)
                float distanciaX = std::abs(barril.posicion.x() - jugador->getPosicion().x());
                float distanciaY = std::abs(barril.posicion.y() - jugador->getPosicion().y());
                float radioTotal = 15.0f * (barril.tipoMovimiento == 3 ? 0.9f : 1.0f) + 15.0f;

                // Fórmula de distancia euclidiana para colisiones circulares
                if ((distanciaX * distanciaX + distanciaY * distanciaY) < (radioTotal * radioTotal)) {
                    float danio = 25.0f;

                    switch(barril.tipoMovimiento) {
                    case 1: danio = 25.0f; break;
                    case 2: danio = 20.0f; break;
                    case 3: danio = 35.0f; break;
                    case 4: danio = 30.0f; break;
                    }

                    danio += qMin(barril.velocidad * 2.0f, 15.0f);

                    jugador->recibirDanio(danio);
                    barril.activo = false;
                    colisionDetectada = true;

                    qDebug() << "💥 COLISIÓN REAL con barril tipo" << barril.tipoMovimiento
                             << "Posición barril:" << barril.posicion
                             << "Posición jugador:" << jugador->getPosicion()
                             << "Distancia:" << std::sqrt(distanciaX*distanciaX + distanciaY*distanciaY)
                             << "Daño:" << danio << "Vida restante:" << jugador->getVida();
                } else {
                    qDebug() << "⚠️ Colisión falsa detectada - distancia:"
                             << std::sqrt(distanciaX*distanciaX + distanciaY*distanciaY)
                             << "radio total:" << radioTotal;
                }
            }
        }
    }

    // Solo reproducir sonido si realmente hubo una colisión
    if (colisionDetectada) {
        AudioManager::getInstance().playPlayerHurt();
    }
}

void Nivel2::limpiarBarriles()
{
    for (int i = barriles.size() - 1; i >= 0; --i) {
        if (!barriles[i].activo || barriles[i].estaFueraDePantalla()) {
            if (barriles[i].estaFueraDePantalla() && barriles[i].activo) {
                barrilesEsquivados++;

                int puntosBase = 10;
                switch(barriles[i].tipoMovimiento) {
                case 1: puntosBase = 10; break;
                case 2: puntosBase = 15; break;
                case 3: puntosBase = 20; break;
                case 4: puntosBase = 18; break;
                }

                int bonusVelocidad = qMin(static_cast<int>(barriles[i].velocidad * 3), 10);
                puntuacion += puntosBase + bonusVelocidad;
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

    dibujarFondoEnCapas(painter);
    dibujarSuelo(painter);
    dibujarBarriles(painter);
    dibujarJugador(painter);
    dibujarHUD(painter);

    if (enPausa) {
        painter.fillRect(rect(), QColor(0, 0, 0, 150));
        UIManager::getInstance().drawText(painter, "PAUSA",
                                          width()/2 - 100, height()/2, 2.0f);
    }

    if (nivelCompletado) {
        painter.fillRect(rect(), QColor(0, 0, 0, 150));

        UIManager::getInstance().drawText(painter, "¡NIVEL COMPLETADO!",
                                          width()/2 - 150, height()/2 - 50, 2.0f);
        UIManager::getInstance().drawText(painter,
                                          QString("Puntuacion: %1").arg(puntuacion),
                                          width()/2 - 100, height()/2, 1.5f);
        UIManager::getInstance().drawText(painter,
                                          QString("Barriles esquivados: %1").arg(barrilesEsquivados),
                                          width()/2 - 120, height()/2 + 50, 1.2f);
    }
}

void Nivel2::dibujarEntidadConSprite(QPainter &painter, const QPointF &posicion,
                                     const QString &spriteName, const QSize &displaySize,
                                     int frameWidth, int frameHeight, int currentFrame)
{
    QPixmap spriteSheet = SpriteManager::getInstance().getSprite(spriteName);

    if(!spriteSheet.isNull()) {
        // Verificar que el sprite esté cargado correctamente
        if (spriteSheet.isNull() || spriteSheet.width() == 0 || spriteSheet.height() == 0) {
            qDebug() << "⚠️ Sprite" << spriteName << "no está cargado correctamente";
            dibujarEntidadSimple(painter, posicion, displaySize, QColor(255, 100, 100));
            return;
        }

        // Calcular cuántos frames caben horizontalmente
        int framesPerRow = spriteSheet.width() / frameWidth;
        if (framesPerRow <= 0) framesPerRow = 1;

        // Calcular fila y columna
        int row = currentFrame / framesPerRow;
        int col = currentFrame % framesPerRow;

        // Verificar que el frame esté dentro de los límites
        int totalFramesVertical = spriteSheet.height() / frameHeight;
        if (row >= totalFramesVertical) {
            row = 0;
            col = 0;
        }

        QRect frameRect(col * frameWidth,
                        row * frameHeight,
                        frameWidth,
                        frameHeight);

        // Verificar que el rectángulo esté dentro del sprite
        if (frameRect.right() > spriteSheet.width() || frameRect.bottom() > spriteSheet.height()) {
            qDebug() << "⚠️ Frame rect fuera de límites, usando frame 0";
            frameRect = QRect(0, 0, frameWidth, frameHeight);
        }

        QPixmap frame = spriteSheet.copy(frameRect);

        QRectF displayRect(posicion.x() - displaySize.width()/2,
                           posicion.y() - displaySize.height()/2,
                           displaySize.width(), displaySize.height());
        painter.drawPixmap(displayRect, frame, frame.rect());

    } else {
        qDebug() << "⚠️ Sprite" << spriteName << "no encontrado, usando fallback";
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
    QPixmap fondo = SpriteManager::getInstance().getSprite("background2");

    if (!fondo.isNull()) {
        painter.drawPixmap(rect(), fondo);
    } else {
        QLinearGradient gradient(0, 0, 0, height());
        gradient.setColorAt(0, QColor(135, 206, 235));
        gradient.setColorAt(1, QColor(100, 180, 220));
        painter.fillRect(rect(), gradient);
    }
}

void Nivel2::dibujarFondoEnCapas(QPainter &painter)
{
    // Capa 1: fondo1 (fondo más lejano)
    QPixmap fondo1 = SpriteManager::getInstance().getSprite("fondo1");
    if (!fondo1.isNull()) {
        // Dibujar fondo1 centrado y ajustado al tamaño de la ventana
        painter.drawPixmap(rect(), fondo1);
    } else {
        // Fallback: gradiente azul
        QLinearGradient gradient(0, 0, 0, height());
        gradient.setColorAt(0, QColor(135, 206, 235));
        gradient.setColorAt(1, QColor(100, 180, 220));
        painter.fillRect(rect(), gradient);
    }

    // Capa 2: fondo2 (fondo intermedio)
    QPixmap fondo2 = SpriteManager::getInstance().getSprite("fondo2");
    if (!fondo2.isNull()) {
        // Dibujar fondo2 con un poco de desplazamiento para efecto de parallax
        // Puedes ajustar el factor de desplazamiento según necesites
        int offsetX = 0; // Cambia esto si quieres efecto parallax
        int offsetY = 0;

        // Ajustar tamaño si es necesario
        QRectF fondo2Rect(offsetX, offsetY, width(), height());
        painter.drawPixmap(fondo2Rect, fondo2, fondo2.rect());

        // Si el sprite es más pequeño que la pantalla, repetirlo
        if (fondo2.width() < width()) {
            int repeticiones = (width() / fondo2.width()) + 1;
            for (int i = 1; i < repeticiones; i++) {
                fondo2Rect = QRectF(offsetX + i * fondo2.width(), offsetY,
                                    fondo2.width(), height());
                painter.drawPixmap(fondo2Rect, fondo2, fondo2.rect());
            }
        }
    }
    // Si no hay fondo2, simplemente no se dibuja nada extra
}

void Nivel2::dibujarSuelo(QPainter &painter)
{
    QPixmap suelo = SpriteManager::getInstance().getSprite("ground2");

    if (!suelo.isNull()) {
        // Asegurar que el sprite se cargó correctamente
        if (suelo.width() == 0 || suelo.height() == 0) {
            qDebug() << "⚠️ Sprite ground2 tiene tamaño 0!";
            // Fallback a color sólido
            painter.setBrush(QColor(34, 139, 34));
            painter.setPen(Qt::NoPen);
            painter.drawRect(0, SUELO_Y, width(), height() - SUELO_Y);
            return;
        }

        int sueloY = SUELO_Y; // 700

        qDebug() << "🎨 Dibujando suelo con sprite ground2 - Tamaño:"
                 << suelo.size() << "Posición Y:" << sueloY;

        // Dibujar suelo como en Nivel3 - sprite repetido sin estirar
        int anchoVista = width();
        int repeticiones = (anchoVista / suelo.width()) + 2;

        for (int i = -1; i < repeticiones; i++) {
            // Dibujar cada tile del suelo manteniendo su proporción
            QRectF destino(i * suelo.width(),
                           sueloY - suelo.height() / 2, // Ajustar para que el sprite se posicione correctamente
                           suelo.width(),
                           suelo.height());
            painter.drawPixmap(destino, suelo, suelo.rect());
        }

        // Añadir línea de separación visual
        painter.setPen(QPen(QColor(0, 100, 0), 2));
        painter.drawLine(0, sueloY, width(), sueloY);

    } else {
        qDebug() << "⚠️ Sprite ground2 NO encontrado, usando fallback";
        // Fallback: suelo verde sólido
        painter.setBrush(QColor(34, 139, 34));
        painter.setPen(Qt::NoPen);
        painter.drawRect(0, SUELO_Y, width(), height() - SUELO_Y);

        // Añadir textura
        painter.setPen(QPen(QColor(0, 100, 0), 1));
        for (int x = 0; x < width(); x += 20) {
            painter.drawLine(x, SUELO_Y, x, height());
        }
    }
}

void Nivel2::dibujarBarriles(QPainter &painter)
{
    // Reemplazar qAsConst con std::as_const
    for (const Barril& barril : std::as_const(barriles)) {
        if (barril.activo) {
            QPointF pos = barril.posicion;
            QSize displaySize = barril.getDisplaySize();

            QPixmap barrilSprite = SpriteManager::getInstance().getSprite("obstacle3");

            if (!barrilSprite.isNull()) {
                // Tinte ligero según tipo
                QPixmap spriteColoreado = barrilSprite;

                if (barril.tipoMovimiento > 1) {
                    QPainter pixmapPainter(&spriteColoreado);
                    pixmapPainter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
                    QColor tint = barril.getColor();
                    tint.setAlpha(60);
                    pixmapPainter.fillRect(spriteColoreado.rect(), tint);
                }

                QRectF displayRect(pos.x() - displaySize.width()/2,
                                   pos.y() - displaySize.height()/2,
                                   displaySize.width(), displaySize.height());
                painter.drawPixmap(displayRect, spriteColoreado, spriteColoreado.rect());
            } else {
                // Fallback
                QColor color = barril.getColor();

                if (barril.tipoMovimiento == 2) {
                    painter.setPen(QPen(color.darker(120), 1, Qt::DashLine));
                    painter.setBrush(QBrush(color));
                }
                else if (barril.tipoMovimiento == 3) {
                    painter.setPen(QPen(color.darker(150), 2));
                    painter.setBrush(QBrush(color.lighter(110)));
                }
                else {
                    painter.setBrush(QBrush(color));
                    painter.setPen(QPen(Qt::white, 2));
                }

                painter.drawEllipse(pos, displaySize.width()/2, displaySize.height()/2);
            }
        }
    }
}

void Nivel2::dibujarJugador(QPainter &painter)
{
    QPointF pos = jugador->getPosicion();

    QString spriteName = jugador->estaMoviendose() ? "player_move" : "player_idle";
    QSize displaySize(60, 80);

    int frameWidth = 192;
    int frameHeight = 192;

    // Usar el frame correcto según el estado
    int currentFrame;
    if (jugador->estaMoviendose()) {
        currentFrame = frameAnimacionMove % 6;  // SIEMPRE entre 0-5
    } else {
        currentFrame = frameAnimacionIdle % 8;  // SIEMPRE entre 0-7
    }

    dibujarEntidadConSprite(painter, pos, spriteName, displaySize, frameWidth, frameHeight, currentFrame);
}

void Nivel2::dibujarHUD(QPainter &painter)
{
    UIManager& ui = UIManager::getInstance();

    const int hudX = 10;
    const int hudY = 10;
    const int panelWidth = 200;
    const int panelHeight = 120;

    if(!ui.getHudPanel().isNull()) {
        painter.drawPixmap(hudX, hudY, panelWidth, panelHeight, ui.getHudPanel());
    } else {
        painter.fillRect(hudX, hudY, panelWidth, panelHeight, QColor(0, 0, 0, 180));
    }

    int panelCenterX = hudX + panelWidth / 2;
    int textY = hudY + 30;
    float scale = 1.0f;
    int lineHeight = 25;

    int tiempoRestante = tiempoObjetivo - tiempoTranscurrido;
    if (tiempoRestante < 0) tiempoRestante = 0;

    QString tiempoText = QString("TIEMPO: %1/%2").arg(tiempoRestante).arg(tiempoObjetivo);
    int tiempoWidth = ui.getTextWidth(tiempoText, scale);
    ui.drawText(painter, tiempoText, panelCenterX - tiempoWidth/2, textY, scale);
    textY += lineHeight;

    QString vidaText = QString("VIDA: %1").arg((int)jugador->getVida());
    int vidaWidth = ui.getTextWidth(vidaText, scale);
    ui.drawText(painter, vidaText, panelCenterX - vidaWidth/2, textY, scale);
    textY += lineHeight;

    float vidaPorcentaje = jugador->getVida() / 100.0f;
    painter.setBrush(QColor(255, 0, 0, 180));
    painter.setPen(Qt::NoPen);
    painter.drawRect(hudX + 10, textY - 10, 180, 12);

    painter.setBrush(QColor(0, 255, 0, 200));
    painter.drawRect(hudX + 10, textY - 10, 180 * vidaPorcentaje, 12);

    // Barra de tiempo
    float progresoTiempo = tiempoTranscurrido / (float)tiempoObjetivo;
    painter.setBrush(QColor(255, 215, 0, 180));
    painter.setPen(Qt::NoPen);
    painter.drawRect(width()/2 - 100, 15, 200 * progresoTiempo, 10);

    painter.setPen(QPen(Qt::white, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(width()/2 - 100, 15, 200, 10);

    // Controles
    int controlesX = width() - 220;
    int controlesY = 30;

    ui.drawText(painter, "CONTROLES:", controlesX, controlesY, 1.0f);
    ui.drawText(painter, "A/← - Izquierda", controlesX, controlesY + 20, 0.9f);
    ui.drawText(painter, "D/→ - Derecha", controlesX, controlesY + 40, 0.9f);
    ui.drawText(painter, "P/ESC - Pausar", controlesX, controlesY + 60, 0.9f);
    ui.drawText(painter, "R - Reanudar", controlesX, controlesY + 80, 0.9f);
}
