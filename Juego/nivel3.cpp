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

    // INICIALIZAR MILESTONES - puedes ajustar estos valores
    milestonesObstaculos = {
        {0.0f, 1},      // Desde el inicio: obstacle1 disponible
        {500.0f, 3},    // A 500px: agregar obstacle3
        {1000.0f, 4},   // A 1000px: agregar obstacle4
        {2000.0f, 2}    // A 2000px: agregar obstacle2 (el más difícil)
    };

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

    // RESTAURAR: Posición original pero un poco más baja
    jugadorN3->setPosicion(QPointF(100, 450)); // Cambiado de 400 a 450 (solo 50px más abajo)

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

int Nivel3::determinarTipoObstaculo(const QRectF& obstaculo)
{
    // Usar posición X como semilla para variedad
    static QRandomGenerator* random = QRandomGenerator::global();
    int semilla = static_cast<int>(obstaculo.x() + obstaculo.y());

    // Generar tipo basado en la posición (siempre el mismo tipo para misma posición)
    return (semilla % 4) + 1; // 1, 2, 3 o 4
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
    QList<int> obstaculosDisponibles = getObstaculosDisponibles();
    float baseX = posicionCamara.x() + width() + 200;

    // Verificar que los obstáculos del patrón estén disponibles
    auto estaDisponible = [&](int tipo) {
        return obstaculosDisponibles.contains(tipo);
    };

    // Función para obtener dimensiones de un tipo
    auto obtenerDimensiones = [](int tipo) -> QPair<QSize, int> {
        switch (tipo) {
        case 1: return {QSize(64, 128), 30};  // obstacle1
        case 2: return {QSize(192, 192), 40}; // obstacle2
        case 3: return {QSize(64, 64), 15};   // obstacle3
        case 4: return {QSize(128, 192), 35}; // obstacle4
        default: return {QSize(64, 128), 30};
        }
    };

    switch (tipoPatron) {
    case 0: // Triple salto
    {
        int tipoUsar = estaDisponible(3) ? 3 : 1;
        auto [dimensiones, ajuste] = obtenerDimensiones(tipoUsar);

        float y = SUELO_Y - dimensiones.height() + ajuste;
        obstaculos.append(QRectF(baseX, y, dimensiones.width(), dimensiones.height()));
        obstaculos.append(QRectF(baseX + 180, y, dimensiones.width(), dimensiones.height()));
        obstaculos.append(QRectF(baseX + 360, y, dimensiones.width(), dimensiones.height()));

        tiposObstaculos.append(tipoUsar);
        tiposObstaculos.append(tipoUsar);
        tiposObstaculos.append(tipoUsar);
    }
    break;

    case 1: // Bajo-Alto-Bajo
    {
        int tipoBajo = estaDisponible(3) ? 3 : 1;
        int tipoAlto = estaDisponible(4) ? 4 : (estaDisponible(1) ? 1 : 3);

        auto [dimBajo, ajusteBajo] = obtenerDimensiones(tipoBajo);
        auto [dimAlto, ajusteAlto] = obtenerDimensiones(tipoAlto);

        float yBajo = SUELO_Y - dimBajo.height() + ajusteBajo;
        float yAlto = SUELO_Y - dimAlto.height() + ajusteAlto;

        obstaculos.append(QRectF(baseX, yBajo, dimBajo.width(), dimBajo.height()));
        obstaculos.append(QRectF(baseX + 200, yAlto, dimAlto.width(), dimAlto.height()));
        obstaculos.append(QRectF(baseX + 400, yBajo, dimBajo.width(), dimBajo.height()));

        tiposObstaculos.append(tipoBajo);
        tiposObstaculos.append(tipoAlto);
        tiposObstaculos.append(tipoBajo);
    }
    break;

    case 2: // Salto largo
    {
        int tipoUsar = estaDisponible(2) ? 2 : (estaDisponible(4) ? 4 : 1);
        auto [dimensiones, ajuste] = obtenerDimensiones(tipoUsar);

        float y = SUELO_Y - dimensiones.height() + ajuste;
        obstaculos.append(QRectF(baseX, y, dimensiones.width(), dimensiones.height()));
        tiposObstaculos.append(tipoUsar);
    }
    break;
    }
}

void Nivel3::generarObstaculosAleatorios()
{
    QRandomGenerator* random = QRandomGenerator::global();
    QList<int> obstaculosDisponibles = getObstaculosDisponibles();

    const int DISTANCIA_MINIMA = 250;
    const int DISTANCIA_MAXIMA = 600;

    // Aumentar cantidad de obstáculos según distancia
    int numObstaculos = 1;
    if (distanciaRecorrida > 1500) numObstaculos = random->bounded(1, 3);
    if (distanciaRecorrida > 3000) numObstaculos = random->bounded(2, 4);

    float ultimaPosicionX = posicionCamara.x() + width() + 150;

    for (int i = 0; i < numObstaculos; i++) {
        int separacion = random->bounded(DISTANCIA_MINIMA, DISTANCIA_MAXIMA);
        float x = ultimaPosicionX + separacion;

        // Elegir solo de los obstáculos disponibles
        int indexTipo = random->bounded(obstaculosDisponibles.size());
        int tipoObstaculo = obstaculosDisponibles[indexTipo];

        // OBTENER DIMENSIONES REALES DEL SPRITE
        QString spriteName;
        QSize dimensionesSprite;
        int ajusteY = 0;

        switch (tipoObstaculo) {
        case 1: // obstacle1: 64x128
            spriteName = "obstacle1";
            dimensionesSprite = QSize(64, 128);
            ajusteY = 30;
            break;
        case 2: // obstacle2: 192x192
            spriteName = "obstacle2";
            dimensionesSprite = QSize(192, 192);
            ajusteY = 40;
            break;
        case 3: // obstacle3: 64x64
            spriteName = "obstacle3";
            dimensionesSprite = QSize(64, 64);
            ajusteY = 15;
            break;
        case 4: // obstacle4: 128x192
            spriteName = "obstacle4";
            dimensionesSprite = QSize(128, 192);
            ajusteY = 35;
            break;
        default:
            spriteName = "obstacle1";
            dimensionesSprite = QSize(64, 128);
            ajusteY = 30;
            break;
        }

        // USAR DIMENSIONES REALES PARA LA HITBOX
        float ancho = dimensionesSprite.width();
        float alto = dimensionesSprite.height();
        float y = SUELO_Y - alto + ajusteY; // Ajustar posición Y

        obstaculos.append(QRectF(x, y, ancho, alto));
        tiposObstaculos.append(tipoObstaculo);
        ultimaPosicionX = x + ancho + 50;

        qDebug() << "🎯 Generado obstáculo tipo" << tipoObstaculo
                 << "Hitbox:" << ancho << "x" << alto << "Pos Y:" << y;
    }

    // Power-ups
    int probPowerUp = 30;
    if (distanciaRecorrida > 2000) probPowerUp = 20;
    if (random->bounded(100) < probPowerUp) {
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
            qDebug() << "💥 COLISIÓN con obstáculo tipo" << tiposObstaculos[i];
            jugadorN3->setVida(jugadorN3->getVida() - 1);
            AudioManager::getInstance().playPlayerHurt();
            obstaculos.removeAt(i);
            tiposObstaculos.removeAt(i); // IMPORTANTE: Remover también el tipo

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
            tiposObstaculos.removeAt(i); // IMPORTANTE: Remover también el tipo
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

    // Fondo del cielo
    painter.fillRect(rect(), QColor(135, 206, 235));

    // Dibujar suelo con sprite
    dibujarSueloConSprite(painter);

    // Resto del código se mantiene igual...
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
    for (int i = 0; i < obstaculos.size(); i++) {
        QRectF obstaculo = obstaculos[i];
        int tipoObstaculo = tiposObstaculos[i];

        QRectF obstaculoVista = obstaculo.translated(-posicionCamara);

        if (obstaculoVista.right() > 0 && obstaculoVista.left() < width()) {
            QString spriteName;

            switch (tipoObstaculo) {
            case 1: spriteName = "obstacle1"; break;
            case 2: spriteName = "obstacle2"; break;
            case 3: spriteName = "obstacle3"; break;
            case 4: spriteName = "obstacle4"; break;
            default: spriteName = "obstacle1"; break;
            }

            QPixmap obstaculoSprite = SpriteManager::getInstance().getSprite(spriteName);

            if (!obstaculoSprite.isNull()) {
                // DIBUJAR DIRECTAMENTE - la hitbox ya coincide con el sprite
                painter.drawPixmap(obstaculoVista, obstaculoSprite, obstaculoSprite.rect());

                // DEBUG: Mostrar hitbox (puedes comentar esto)
                painter.setBrush(Qt::NoBrush);
                painter.setPen(QPen(Qt::red, 1, Qt::DashLine));
                painter.drawRect(obstaculoVista);

            } else {
                // Fallback
                painter.setBrush(QBrush(QColor(150, 75, 0)));
                painter.setPen(QPen(Qt::black, 2));
                painter.drawRect(obstaculoVista);
            }
        }
    }
}

void Nivel3::dibujarJugador(QPainter &painter)
{
    // RESTAURAR: Usar la posición real del jugador como estaba originalmente
    float jugadorY = jugadorN3->getPosicion().y();

    QString spriteName;
    int frameIndex = frameAnimacion;
    int frameWidth, frameHeight;
    QSize displaySize(80, 100); // Tamaño de visualización en pantalla

    // Determinar qué sprite usar según el estado (ORIGINAL)
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
        QRect frameRect(frameIndex * frameWidth, 0, frameWidth, frameHeight);
        QPixmap frame = spriteSheet.copy(frameRect);

        QRectF displayRect(100 - displaySize.width()/2,
                           jugadorY - displaySize.height()/2,
                           displaySize.width(),
                           displaySize.height());
        painter.drawPixmap(displayRect, frame, frame.rect());

        static int debugCounter = 0;
        if (debugCounter++ % 120 == 0) {
            qDebug() << " Sprite:" << spriteName
                     << "Frame:" << frameIndex
                     << "Pos Y:" << jugadorY
                     << "Estado - Saltando:" << jugadorN3->estaSaltando
                     << "Agachado:" << jugadorN3->estaAgachado;
        }

    } else {
        painter.setBrush(QBrush(QColor(0, 100, 200)));
        painter.setPen(QPen(Qt::white, 3));
        QRectF jugadorRect(80, jugadorY - 25, 40, 50);
        painter.drawRect(jugadorRect);
        qDebug() << "Sprite no encontrado:" << spriteName;
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

    // Barra de tiempo - RESTAURAR RIBBON ROJO
    float progresoTiempo = tiempoRestante / tiempoObjetivo;
    QPixmap ribbon = UIManager::getInstance().getRibbonRed(); // RIBBON ROJO RESTAURADO
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
