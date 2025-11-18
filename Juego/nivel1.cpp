#include "nivel1.h"
#include "spritemanager.h"
#include <QPainter>
#include <QDebug>
#include <QtMath>
#include <QInputDialog>
#include <QMessageBox>
#include <QApplication>

Nivel1::Nivel1(QWidget *parent) : QWidget(parent)
{
    // Configurar ventana
    setFixedSize(1300, 730);
    setFocusPolicy(Qt::StrongFocus);

    SpriteManager::getInstance().preloadGameSprites();

    // Inicializar mapa con PNG
    mapa = new Mapa(this);

    // Cargar tu mapa PNG
    bool mapaCargado = mapa->cargarMapaDesdePNG(":/map/maps/map1.png");

    if(!mapaCargado) {
        qDebug() << "No se pudo cargar mapa PNG, usando mapa básico...";
        mapa->crearMapaBasico();
    }

    // Inicializar jugador en posición del mapa
    jugador = new JugadorNivel1();
    jugador->setPosicion(mapa->getPosicionInicioJugador());

    // Inicializar timers
    timerJuego = new QTimer(this);
    timerOleadas = new QTimer(this);
    tiempoUltimoFrame = QDateTime::currentMSecsSinceEpoch();

    connect(timerJuego, &QTimer::timeout, this, &Nivel1::actualizarJuego);
    connect(timerOleadas, &QTimer::timeout, this, &Nivel1::generarOleada);

    // INICIALIZAR TODAS LAS VARIABLES
    for(int i = 0; i < 4; i++) {
        teclas[i] = false;
    }

    tiempoTranscurrido = 0;
    tiempoObjetivo = 120;
    numeroOleada = 1;
    enemigosPorOleada = 2;
    frecuenciaGeneracion = 5000;

    mostrandoMejoras = false;
    opcionesMejorasActuales = QList<Mejora>();

    // Inicializar sistema de mejoras
    inicializarMejoras();

    iniciarNivel();
}

Nivel1::~Nivel1()
{
    qDeleteAll(enemigos);
    enemigos.clear();
    delete jugador;
    delete mapa;
}

void Nivel1::iniciarNivel()
{
    timerJuego->start(16); // ~60 FPS
    timerOleadas->start(frecuenciaGeneracion);
}

void Nivel1::resetearTeclas() {
    for(int i = 0; i < 4; i++) {
        teclas[i] = false;
    }
    if(jugador) {
        bool teclasVacias[4] = {false, false, false, false};
        jugador->procesarInput(teclasVacias);
    }
}

void Nivel1::pausarNivel()
{
    timerJuego->stop();
    timerOleadas->stop();
    resetearTeclas();
    emit gamePaused();  // EMITIR SEÑAL
}

void Nivel1::reanudarNivel()
{
    timerJuego->start(16);
    timerOleadas->start(frecuenciaGeneracion);
    emit gameResumed();  // EMITIR SEÑAL
}

void Nivel1::inicializarMejoras()
{
    todasLasMejoras.clear();

    todasLasMejoras.append(Mejora(Mejora::ARMA, "Espada Bizantina",
                                  "Corte direccional frente al jugador", Arma::ESPADA));

    todasLasMejoras.append(Mejora(Mejora::ARMA, "Ballesta",
                                  "Dispara flechas en dirección al movimiento", Arma::BALLESTA));

    todasLasMejoras.append(Mejora(Mejora::ARMA, "Arco Multidireccional",
                                  "Dispara en 4 direcciones simultáneas", Arma::ARCO));

    todasLasMejoras.append(Mejora(Mejora::ARMA, "Aceite Hirviendo",
                                  "Área de daño alrededor del jugador", Arma::ACEITE));

}

QList<Mejora> Nivel1::generarOpcionesMejoras(int cantidad)
{
    QList<Mejora> opciones;
    QList<Mejora> disponibles = todasLasMejoras;

    // Filtrar armas NUEVAS que el jugador no tiene
    QList<Mejora> armasNuevas;
    for(int i = disponibles.size() - 1; i >= 0; i--) {
        if(!jugador->tieneArma(static_cast<Arma::Tipo>(disponibles[i].getTipoArma()))) {
            armasNuevas.append(disponibles[i]);
        }
    }

    // Si hay menos armas nuevas de las solicitadas, completar con mejoras de armas existentes
    if(armasNuevas.size() < cantidad) {
        // Crear mejoras para armas que el jugador YA tiene
        QList<Arma*> armasJugador = jugador->getArmas();

        for(Arma* arma : armasJugador) {
            if(armasNuevas.size() >= cantidad) break;

            QString nombreMejora = QString("Mejorar %1").arg(arma->getNombre());
            QString descripcion = QString("Aumenta el nivel de %1").arg(arma->getNombre());

            // Crear una mejora especial para subir nivel
            Mejora mejora(Mejora::ARMA, nombreMejora, descripcion, arma->getTipo());
            armasNuevas.append(mejora);
        }
    }

    // Mezclar y seleccionar
    for(int i = armasNuevas.size() - 1; i > 0; --i) {
        int j = QRandomGenerator::global()->bounded(i + 1);
        armasNuevas.swapItemsAt(i, j);
    }

    int numOpciones = qMin(cantidad, armasNuevas.size());
    for(int i = 0; i < numOpciones; ++i) {
        opciones.append(armasNuevas[i]);
    }

    return opciones;
}

void Nivel1::aplicarMejora(const Mejora& mejora)
{
    qDebug() << "=== INICIANDO APLICACIÓN DE MEJORA ===";
    qDebug() << "Nombre:" << mejora.getNombre();
    qDebug() << "Tipo arma:" << mejora.getTipoArma();

    // Verificar qué armas tiene antes
    qDebug() << "ARMAS ANTES de aplicar mejora:";
    for(Arma* arma : jugador->getArmas()) {
        qDebug() << " - " << arma->getNombre() << "(Tipo:" << arma->getTipo() << ")";
    }

    mejora.aplicar(jugador);

    // Verificar qué armas tiene después
    qDebug() << "ARMAS DESPUÉS de aplicar mejora:";
    for(Arma* arma : jugador->getArmas()) {
        qDebug() << " - " << arma->getNombre() << "(Tipo:" << arma->getTipo() << ")";
    }

    // Mostrar confirmación visual
    QString mensaje = QString("¡%1 obtenido!\n%2")
                          .arg(mejora.getNombre())
                          .arg(mejora.getDescripcion());

    QMessageBox::information(this, "¡Mejora Aplicada!", mensaje);
    qDebug() << "✅ MEJORA APLICADA CORRECTAMENTE:" << mejora.getNombre();
}

void Nivel1::mostrarOpcionesMejoras()
{
    mostrandoMejoras = true;
    resetearTeclas();
    pausarNivel();

    // GUARDAR las opciones en la variable de clase
    opcionesMejorasActuales = generarOpcionesMejoras(3);

    // DEBUG: Mostrar qué opciones se generaron
    qDebug() << "=== OPCIONES GENERADAS Y GUARDADAS ===";
    for(int i = 0; i < opcionesMejorasActuales.size(); ++i) {
        qDebug() << i << "->" << opcionesMejorasActuales[i].getNombre()
        << "Arma:" << opcionesMejorasActuales[i].getTipoArma();
    }

    QString mensaje = "¡Subiste de nivel!\nElige un arma:\n\n";
    for(int i = 0; i < opcionesMejorasActuales.size(); ++i) {
        const Mejora& mejora = opcionesMejorasActuales[i];
        mensaje += QString("%1. 🎯 %2\n   %3\n")
                       .arg(i + 1)
                       .arg(mejora.getNombre())
                       .arg(mejora.getDescripcion());
    }
    mensaje += "\nPresiona 1, 2 o 3 para elegir";

    QMessageBox::information(this, "¡Nueva Arma Disponible!", mensaje);
}

void Nivel1::actualizarJuego()
{
    // Calcular deltaTime
    qint64 tiempoActual = QDateTime::currentMSecsSinceEpoch();
    float deltaTime = tiempoActual - tiempoUltimoFrame;
    tiempoUltimoFrame = tiempoActual;

    if(mostrandoMejoras) {
        return;
    }

    // Actualizar tiempo del juego
    static float acumuladorTiempo = 0;
    acumuladorTiempo += deltaTime;
    if(acumuladorTiempo >= 1000) {
        tiempoTranscurrido++;
        acumuladorTiempo = 0;
    }

    // Guardar posición anterior para colisiones
    QPointF posicionAnterior = jugador->getPosicion();

    // Actualizar jugador
    jugador->procesarInput(teclas);
    jugador->actualizar(deltaTime);

    // NUEVO: Verificar colisiones con el mapa
    QRectF areaJugador = jugador->getAreaColision();
    if(!verificarColisionMapa(areaJugador)) {
        // Hay colisión, revertir movimiento
        jugador->setPosicion(posicionAnterior);
    }

    // Actualizar enemigos con colisiones
    for(Enemigo *enemigo : enemigos) {
        QPointF posicionAnteriorEnemigo = enemigo->getPosicion();
        enemigo->seguirJugador(jugador->getPosicion());
        enemigo->actualizar(deltaTime);

        // Verificar colisión con mapa
        if(!verificarColisionMapa(enemigo->getAreaColision())) {
            enemigo->setPosicion(posicionAnteriorEnemigo);
        }
    }

    // El resto del método se mantiene igual...
    procesarColisiones();
    limpiarEnemigosMuertos();

    if(jugador->tieneMejoraPendiente() && !mostrandoMejoras) {
        mostrarOpcionesMejoras();
    }

    if(tiempoTranscurrido >= tiempoObjetivo) {
        timerJuego->stop();
        timerOleadas->stop();
        qDebug() << "¡Nivel completado! Has sobrevivido 2 minutos";
        emit levelCompleted();
    }

    if(!jugador->estaViva()) {
        timerJuego->stop();
        timerOleadas->stop();
        qDebug() << "Game Over - Has sido derrotado";
        emit gameOver();
    }

    update();
}

bool Nivel1::verificarColisionMapa(const QRectF& area) const
{
    if(!mapa) return true;

    // Verificar cada esquina y centro del área
    QPointF puntos[] = {
        area.topLeft(),
        area.topRight(),
        area.bottomLeft(),
        area.bottomRight(),
        area.center()
    };

    for(const QPointF& punto : puntos) {
        if(!mapa->esTransitable(punto.x(), punto.y())) {
            return false; // Hay colisión
        }
    }

    // Verificar límites del mapa
    QRectF limites = mapa->getLimitesMapa();
    if(!limites.contains(area)) {
        return false;
    }

    return true; // No hay colisión
}

void Nivel1::generarOleada()
{
    int enemigosBase = 2 + (numeroOleada / 3);
    int enemigosExtra = QRandomGenerator::global()->bounded(2);

    for(int i = 0; i < enemigosBase + enemigosExtra; i++) {
        generarEnemigo();
    }

    numeroOleada++;
    frecuenciaGeneracion = qMax(1500, frecuenciaGeneracion - 50); // Más gradual
    timerOleadas->setInterval(frecuenciaGeneracion);
}

void Nivel1::generarEnemigo()
{
    // Determinar tipo de enemigo según oleada
    int tipoEnemigo;
    int probabilidadFuerte = qMin(20 + (numeroOleada - 1) * 10, 60);

    if(QRandomGenerator::global()->bounded(100) < probabilidadFuerte) {
        tipoEnemigo = 2; // Fuerte
    } else {
        tipoEnemigo = 1; // Débil
    }

    // Crear enemigo con el tipo específico
    Enemigo *enemigo = new Enemigo(tipoEnemigo);

    // Generar en posición aleatoria en los bordes
    int lado = QRandomGenerator::global()->bounded(4);
    QPointF posicion;

    switch(lado) {
    case 0: // Arriba
        posicion = QPointF(QRandomGenerator::global()->bounded(1300), -20);
        break;
    case 1: // Derecha
        posicion = QPointF(1320, QRandomGenerator::global()->bounded(730));
        break;
    case 2: // Abajo
        posicion = QPointF(QRandomGenerator::global()->bounded(1300), 750);
        break;
    case 3: // Izquierda
        posicion = QPointF(-20, QRandomGenerator::global()->bounded(730));
        break;
    }

    enemigo->setPosicion(posicion);
    enemigos.append(enemigo);

    qDebug() << "🔥 ENEMIGO GENERADO - Tipo:" << tipoEnemigo
             << "Pos:" << posicion
             << "Total enemigos:" << enemigos.size();
}

void Nivel1::procesarColisiones()
{
    QHash<Enemigo*, bool> enemigosGolpeadosPorArmas;

    for(Arma* arma : jugador->getArmas()) {
        QList<QRectF> areasAtaque = arma->getAreasAtaque();

        for(const QRectF& area : areasAtaque) {
            for(Enemigo* enemigo : enemigos) {
                if(enemigo->estaViva() &&
                    !enemigosGolpeadosPorArmas.value(enemigo, false) &&
                    area.intersects(enemigo->getAreaColision())) {

                    enemigo->recibirDanio(arma->getDanio());
                    enemigosGolpeadosPorArmas[enemigo] = true;

                    if(!enemigo->estaViva()) {
                        jugador->ganarExperiencia(enemigo->getExperienciaQueDa());
                    }
                }
            }
        }
    }

    for(Enemigo* enemigo : enemigos) {
        if(enemigo->estaViva() &&
            jugador->getAreaColision().intersects(enemigo->getAreaColision())) {
            jugador->recibirDanio(0.5f); // Daño por frame de contacto
        }
    }
}

void Nivel1::limpiarEnemigosMuertos()
{
    for(int i = enemigos.size() - 1; i >= 0; i--) {
        if(!enemigos[i]->estaViva()) {
            delete enemigos[i];
            enemigos.removeAt(i);
        }
    }
}

void Nivel1::keyPressEvent(QKeyEvent *event)
{
    if(mostrandoMejoras) {
        if(event->key() >= Qt::Key_1 && event->key() <= Qt::Key_3) {
            int indice = event->key() - Qt::Key_1; // Esto da 0, 1 o 2

            // USAR las opciones guardadas, NO regenerarlas
            qDebug() << "Selección tecla:" << indice;
            qDebug() << "Opciones disponibles:" << opcionesMejorasActuales.size();

            if(indice >= 0 && indice < opcionesMejorasActuales.size()) {
                const Mejora& mejoraSeleccionada = opcionesMejorasActuales[indice];
                qDebug() << "✅ Aplicando mejora seleccionada:" << mejoraSeleccionada.getNombre()
                         << "Tipo arma:" << mejoraSeleccionada.getTipoArma();

                aplicarMejora(mejoraSeleccionada);
                jugador->setMejoraPendiente(false);
                mostrandoMejoras = false;

                // Limpiar las opciones actuales
                opcionesMejorasActuales.clear();

                resetearTeclas();
                reanudarNivel();
            } else {
                qDebug() << "❌ Índice inválido:" << indice;
                qDebug() << "Opciones disponibles:" << opcionesMejorasActuales.size();
            }
        } else {
            qDebug() << "Tecla presionada durante mejora:" << event->key();
        }
        return;
    }

    // Teclas normales del juego...
    switch(event->key()) {
    case Qt::Key_W: teclas[0] = true; break;
    case Qt::Key_A: teclas[1] = true; break;
    case Qt::Key_S: teclas[2] = true; break;
    case Qt::Key_D: teclas[3] = true; break;
    case Qt::Key_P: pausarNivel(); break;
    case Qt::Key_R: reanudarNivel(); break;
    }
}

void Nivel1::keyReleaseEvent(QKeyEvent *event)
{
    switch(event->key()) {
    case Qt::Key_W: teclas[0] = false; break;
    case Qt::Key_A: teclas[1] = false; break;
    case Qt::Key_S: teclas[2] = false; break;
    case Qt::Key_D: teclas[3] = false; break;
    }
}

void Nivel1::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // NUEVO: Dibujar fondo temporal
    painter.fillRect(rect(), QBrush(QColor(60, 60, 80))); // Fondo gris oscuro

    // DEBUG: Dibujar texto de diagnóstico
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 14));
    painter.drawText(10, 30, "Diagnóstico del Mapa:");

    if(mapa) {
        painter.drawText(10, 60, "Mapa: CREADO");
        painter.drawText(10, 90, QString("Tamaño: %1x%2 tiles").arg(mapa->getTamanoMapa().width()).arg(mapa->getTamanoMapa().height()));
    } else {
        painter.drawText(10, 60, "Mapa: NULL");
    }

    // Intentar dibujar el mapa
    QRectF vista(0, 0, width(), height());
    if(mapa) {
        mapa->dibujar(painter, vista);
    }

    // El resto de tu código de dibujo (jugador, enemigos, etc.)
    QPointF playerPos = jugador->getPosicion();

    // DEBUG: Dibujar posición del jugador
    painter.setPen(Qt::yellow);
    painter.drawText(10, 120, QString("Jugador: %1, %2").arg(playerPos.x()).arg(playerPos.y()));
    // === VARIABLES DE ANIMACIÓN (declaradas al inicio) ===
    static int currentFrame = 0;
    static int animationCounter = 0;
    static int debugCounter = 0;

    // Determinar qué sprite usar según si se está moviendo
    bool isMoving = (teclas[0] || teclas[1] || teclas[2] || teclas[3]); // W, A, S, D presionadas

    QPixmap playerSpriteSheet;
    if(isMoving) {
        playerSpriteSheet = SpriteManager::getInstance().getSprite("player_move");
    } else {
        playerSpriteSheet = SpriteManager::getInstance().getSprite("player_idle");
    }

    if(!playerSpriteSheet.isNull()) {
        // Dimensiones para sprite de 1152x192 con 6 frames
        int frameWidth = 192;   // 1152 ÷ 6 = 192
        int frameHeight = 192;  // Alto completo

        // Animación: cambiar frame según el tiempo
        animationCounter++;

        // Cambiar frame cada 8 frames del juego (ajustable)
        if(animationCounter >= 8) {
            currentFrame = (currentFrame + 1) % 6; // 6 frames totales, ciclo 0-5
            animationCounter = 0;
        }

        // Extraer el frame actual de la sprite sheet
        QRect frameRect(currentFrame * frameWidth, 0, frameWidth, frameHeight);
        QPixmap frame = playerSpriteSheet.copy(frameRect);

        // Dibujar el frame escalado a tamaño adecuado para el juego
        int displayWidth = 52;   // Tamaño en pantalla
        int displayHeight = 52;
        QRectF displayRect(playerPos.x() - displayWidth/2,
                           playerPos.y() - displayHeight/2,
                           displayWidth, displayHeight);
        painter.drawPixmap(displayRect, frame, frame.rect());

    } else {
        // Fallback: dibujar círculo azul si no hay sprite
        painter.setBrush(QBrush(QColor(0, 100, 255)));
        painter.setPen(QPen(Qt::white, 2));
        painter.drawEllipse(playerPos, 15, 15);
    }

    int enemyFrame = 0;
    int enemyAnimCounter = 0;

    for(Enemigo *enemigo : enemigos) {
        if(enemigo->estaViva()) {
            QPointF enemyPos = enemigo->getPosicion();
            int enemyType = enemigo->getTipo();

            // DEBUG: Verificar que los enemigos existen
            static int debugCounter = 0;
            if(debugCounter % 180 == 0) { // Cada 3 segundos aprox
                qDebug() << "Enemigos activos:" << enemigos.size()
                         << "Tipo:" << enemyType << "Pos:" << enemyPos;
            }
            debugCounter++;

            // Seleccionar sprite según el tipo
            QString spriteName = (enemyType == 1) ? "enemy_weak" : "enemy_strong";
            QPixmap enemySpriteSheet = SpriteManager::getInstance().getSprite(spriteName);

            if(!enemySpriteSheet.isNull()) {
                // Animación simple basada en tiempo
                enemyFrame = (QDateTime::currentMSecsSinceEpoch() / 150) % 6; // 6 frames

                // Configuración específica por tipo
                int frameWidth, frameHeight;
                int displayWidth, displayHeight;

                if(enemyType == 1) {
                    // ENEMIGO DÉBIL: 1920x320 con 6 frames
                    frameWidth = 320;   // 1920 ÷ 6 = 320
                    frameHeight = 320;  // Alto completo
                    displayWidth = 42;  // Tamaño en pantalla
                    displayHeight = 42;
                } else {
                    // ENEMIGO FUERTE: 1152x192 con 6 frames
                    frameWidth = 192;   // 1152 ÷ 6 = 192
                    frameHeight = 192;  // Alto completo
                    displayWidth = 52;  // Más grande
                    displayHeight = 52;
                }

                // Extraer frame actual
                QRect frameRect(enemyFrame * frameWidth, 0, frameWidth, frameHeight);
                QPixmap frame = enemySpriteSheet.copy(frameRect);

                // Dibujar enemigo
                QRectF displayRect(enemyPos.x() - displayWidth/2,
                                   enemyPos.y() - displayHeight/2,
                                   displayWidth, displayHeight);
                painter.drawPixmap(displayRect, frame, frame.rect());

            } else {
                // Fallback con colores diferentes
                qDebug() << "Sprite no encontrado para:" << spriteName;
                if(enemyType == 1) {
                    painter.setBrush(QBrush(QColor(255, 100, 100))); // Rojo claro
                    painter.drawEllipse(enemyPos, 12, 12);
                } else {
                    painter.setBrush(QBrush(QColor(180, 50, 50)));   // Rojo oscuro
                    painter.drawEllipse(enemyPos, 16, 16);
                }
                painter.setPen(QPen(Qt::white, 1));
            }
        }
    }

    // Dibujar armas y ataques
    dibujarArmas(painter);

    // Dibujar HUD
    dibujarHUD(painter);

    // Debug: mostrar información de animación (opcional)
    debugCounter++;
    if(debugCounter % 120 == 0) { // Cada 2 segundos aprox
        qDebug() << "Animación - Frame:" << currentFrame << "Movimiento:" << isMoving;
    }
}

void Nivel1::dibujarArmas(QPainter &painter)
{
    for(Arma* arma : jugador->getArmas()) {

        // 1. DIBUJAR PROYECTILES (Ballesta y Arco) - MISMO SPRITE
        QList<Arma::ProyectilSprite> proyectiles = arma->getProyectilesSprites();
        for(const auto& proyectil : proyectiles) {
            // USAR EL MISMO SPRITE PARA AMBAS ARMAS
            QPixmap sprite = SpriteManager::getInstance().getSprite("projectile_arrow");

            if(!sprite.isNull()) {
                painter.save();
                painter.translate(proyectil.posicion);
                painter.rotate(proyectil.rotacion);

                // Tamaño del proyectil
                QSize displaySize(16, 16);
                QRectF destRect(-displaySize.width()/2, -displaySize.height()/2,
                                displaySize.width(), displaySize.height());
                painter.drawPixmap(destRect, sprite, sprite.rect());

                painter.restore();
            } else {
                // Fallback: círculos de colores diferentes para distinguir
                if(arma->getTipo() == Arma::BALLESTA) {
                    painter.setBrush(QBrush(QColor(139, 69, 19))); // Marrón para ballesta
                } else {
                    painter.setBrush(QBrush(QColor(160, 82, 45))); // Marrón claro para arco
                }
                painter.drawEllipse(proyectil.posicion, 4, 4);
            }
        }

        // 2. DIBUJAR ATAQUES DE ÁREA (Espada y Aceite)
        QList<Arma::AreaAtaqueSprite> areas = arma->getAreasAtaqueSprites();
        for(const auto& areaSprite : areas) {
            if(arma->getTipo() == Arma::ESPADA) {
                dibujarAtaqueEspada(painter, arma, areaSprite);
            } else if(arma->getTipo() == Arma::ACEITE) {
                dibujarAtaqueAceite(painter, arma, areaSprite);
            }
        }
    }
}

void Nivel1::dibujarAtaqueEspada(QPainter &painter, Arma* arma, const Arma::AreaAtaqueSprite& areaSprite)
{
    QPixmap spriteFrame = SpriteManager::getInstance().getSpriteFrame(
        areaSprite.spriteName,
        areaSprite.frameActual % areaSprite.totalFrames
        );

    if(!spriteFrame.isNull()) {
        painter.save();

        // Posicionar en el centro del área de ataque
        QPointF centro = areaSprite.area.center();
        painter.translate(centro);

        // Aplicar rotación según la dirección del ataque
        painter.rotate(areaSprite.rotacion);

        // Escalar el sprite para que coincida con el área de ataque
        QSizeF spriteSize = areaSprite.area.size();
        QRectF destRect(-spriteSize.width()/2, -spriteSize.height()/2,
                        spriteSize.width(), spriteSize.height());

        painter.drawPixmap(destRect, spriteFrame, spriteFrame.rect());
        painter.restore();

    } else {
        // Fallback: dibujo geométrico
        qDebug() << "Sprite de espada no encontrado, usando fallback";
        QColor colorArma = arma->getColor();
        painter.setBrush(QBrush(colorArma.lighter(150), Qt::Dense4Pattern));
        painter.setPen(QPen(colorArma.darker(130), 2));
        painter.drawRect(areaSprite.area);
    }
}

void Nivel1::dibujarAtaqueAceite(QPainter &painter, Arma* arma, const Arma::AreaAtaqueSprite& areaSprite)
{
    QPixmap spriteFrame = SpriteManager::getInstance().getSpriteFrame(
        areaSprite.spriteName,
        areaSprite.frameActual % areaSprite.totalFrames
        );

    if(!spriteFrame.isNull()) {
        painter.save();

        QPointF centro = areaSprite.area.center();
        painter.translate(centro);

        // Para el aceite, usar el tamaño del área pero mantener la relación de aspecto
        float radio = areaSprite.area.width() / 2;
        QRectF destRect(-radio, -radio, radio * 2, radio * 2);

        painter.drawPixmap(destRect, spriteFrame, spriteFrame.rect());
        painter.restore();

        // DEBUG: Mostrar información del aceite (opcional)
        static int oilDebugCounter = 0;
        if(oilDebugCounter % 90 == 0) {
            qDebug() << "Aceite - Frame:" << (areaSprite.frameActual % areaSprite.totalFrames)
            << "/" << areaSprite.totalFrames
            << "Área:" << areaSprite.area
            << "Tiempo vida:" << areaSprite.tiempoVida;
        }
        oilDebugCounter++;

    } else {
        // Fallback: dibujo geométrico
        qDebug() << "❌ Sprite de aceite no encontrado, usando fallback geométrico";
        QColor colorArma = arma->getColor();

        // Dibujo más elaborado para el fallback
        painter.setBrush(QBrush(colorArma, Qt::DiagCrossPattern));
        painter.setPen(QPen(colorArma.darker(), 2));
        painter.drawEllipse(areaSprite.area);

        // Efecto de burbujas
        painter.setBrush(QBrush(colorArma.lighter(120)));
        painter.setPen(Qt::NoPen);

        // Dibujar algunas burbujas aleatorias
        for(int i = 0; i < 5; i++) {
            float offsetX = (QRandomGenerator::global()->bounded(200) - 100) / 100.0f * areaSprite.area.width() / 3;
            float offsetY = (QRandomGenerator::global()->bounded(200) - 100) / 100.0f * areaSprite.area.height() / 3;
            QPointF bubblePos = areaSprite.area.center() + QPointF(offsetX, offsetY);
            float bubbleSize = QRandomGenerator::global()->bounded(3) + 2;
            painter.drawEllipse(bubblePos, bubbleSize, bubbleSize);
        }
    }
}

void Nivel1::dibujarHUD(QPainter &painter)
{
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 12));

    painter.drawText(10, 20, QString("Vida: %1").arg(jugador->getVida()));
    painter.drawText(10, 40, QString("Nivel: %1").arg(jugador->getNivel()));
    painter.drawText(10, 60, QString("EXP: %1/%2").arg(jugador->getExperiencia())
                                 .arg(jugador->getExperienciaParaSiguienteNivel()));

    painter.drawText(10, 80, QString("Tiempo: %1/%2").arg(tiempoTranscurrido)
                                 .arg(tiempoObjetivo));
    painter.drawText(10, 100, QString("Oleada: %1").arg(numeroOleada));
    painter.drawText(10, 120, QString("Enemigos: %1").arg(enemigos.size()));

    int yPos = 140;
    painter.drawText(10, yPos, "Armas Activas:");
    yPos += 20;

    for(Arma* arma : jugador->getArmas()) {
        painter.drawText(20, yPos, QString("- %1").arg(arma->getNombre()));
        yPos += 15;
    }

    painter.drawText(10, 560, "Controles: WASD - Movimiento");
    painter.drawText(10, 580, "P - Pausa, R - Reanudar");

    if(jugador->tieneMejoraPendiente()) {
        painter.setPen(Qt::yellow);
        painter.drawText(300, 30, "¡ARMA NUEVA DISPONIBLE! Presiona M para elegir");
    }

    if(mostrandoMejoras) {
        painter.fillRect(rect(), QColor(0, 0, 0, 150));
        painter.setPen(Qt::yellow);
        painter.setFont(QFont("Arial", 16, QFont::Bold));
        painter.drawText(rect().center() - QPoint(150, 0), "ELIGE UN ARMA (1, 2, 3)");
    }
}

void Nivel1::dibujarBarraVidaEnemigo(QPainter &painter, Enemigo *enemigo, const QPointF &posicion)
{
    float vidaPorcentaje = enemigo->getVida() / (enemigo->getTipo() == 1 ? 30.0f : 60.0f);

    // Barra de vida sobre el enemigo
    QRectF barraFondo(posicion.x() - 15, posicion.y() - 30, 30, 4);
    QRectF barraVida(posicion.x() - 15, posicion.y() - 30, 30 * vidaPorcentaje, 4);

    painter.setBrush(QBrush(QColor(50, 50, 50)));
    painter.setPen(Qt::NoPen);
    painter.drawRect(barraFondo);

    // Color según tipo
    if(enemigo->getTipo() == 1) {
        painter.setBrush(QBrush(QColor(255, 100, 100))); // Rojo claro para débil
    } else {
        painter.setBrush(QBrush(QColor(255, 50, 50)));   // Rojo oscuro para fuerte
    }
    painter.drawRect(barraVida);
}
