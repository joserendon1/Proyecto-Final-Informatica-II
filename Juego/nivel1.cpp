#include "nivel1.h"
#include "spritemanager.h"
#include "uimanager.h"
#include "audiomanager.h"
#include <QTimer>
#include <QPainter>
#include <QDebug>
#include <QtMath>
#include <QMessageBox>
#include <QApplication>
#include <algorithm>
#include <QRandomGenerator>

Nivel1::Nivel1(QWidget *parent) : QWidget(parent)
{
    setFixedSize(1024, 768);
    tamanoVista = QSize(1024, 768);

    qDebug() << "Nivel1 - Vista 1024x768";
    setFocusPolicy(Qt::StrongFocus);

    // PRIMERO cargar recursos
    SpriteManager::getInstance().preloadGameSprites();
    UIManager::getInstance().loadResources();

    qDebug() << "Cargando audio...";
    bool audioOk = AudioManager::getInstance().loadSounds();
    qDebug() << "Audio cargado:" << audioOk;

    if (audioOk) {
        AudioManager::getInstance().playBackgroundMusic();
        qDebug() << "Música iniciada";
    }

    // *** CORRECCIÓN: PRIMERO el mapa, LUEGO el jugador ***
    mapa = new Mapa(this);

    // INICIALIZAR el mapa completamente
    inicializarMapaGrande();

    // AHORA crear el jugador cuando el mapa ya está listo
    jugador = new JugadorNivel1();

    // Configurar posición y cámara
    QPointF posicionInicial = mapa->getPosicionInicioJugador();
    jugador->setPosicion(posicionInicial);
    posicionCamara = posicionInicial - QPointF(tamanoVista.width()/2, tamanoVista.height()/2);

    // Resto de la inicialización...
    timerJuego = new QTimer(this);
    timerOleadas = new QTimer(this);
    tiempoUltimoFrame = QDateTime::currentMSecsSinceEpoch();

    connect(timerJuego, &QTimer::timeout, this, &Nivel1::actualizarJuego);
    connect(timerOleadas, &QTimer::timeout, this, &Nivel1::generarOleada);

    for(int i = 0; i < 4; i++) {
        teclas[i] = false;
    }

    tiempoTranscurrido = 0;
    tiempoObjetivo = 120;
    numeroOleada = 1;
    enemigosPorOleada = 4;
    frecuenciaGeneracion = 3500;

    mostrandoMejoras = false;
    opcionSeleccionada = 0;
    opcionesMejorasActuales = QList<Mejora>();

    inicializarMejoras();

    qDebug() << "Nivel1 completamente inicializado - iniciando nivel...";
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
    timerJuego->start(16);
    timerOleadas->start(frecuenciaGeneracion);
}

void Nivel1::inicializarMapaGrande()
{
    QSize tamanoMapa(1024 * 5, 768 * 5);
    qDebug() << "🗺️ Creando mapa 5120x3840 (5x 1024x768)";

    mapa->crearMapaGrande(tamanoMapa);

    QPointF posInicial = mapa->getPosicionInicioJugador();
    QRectF areaJugador = QRectF(posInicial.x() - 10, posInicial.y() - 10, 20, 20);
    bool esValida = verificarColisionMapa(areaJugador);

    qDebug() << "Posición inicial verificada:" << posInicial << "- Válida:" << esValida;
    qDebug() << "MAPA 5x LISTO - Área enorme para explorar!";
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
    emit gamePaused();
}

void Nivel1::reanudarNivel()
{
    timerJuego->start(16);
    timerOleadas->start(frecuenciaGeneracion);
    emit gameResumed();
}

void Nivel1::inicializarMejoras()
{
    todasLasMejoras.clear();

    todasLasMejoras.append(Mejora(Mejora::ARMA, "Ballesta",
                                  "Dispara flechas en dirección al enemigo más cercano", Arma::BALLESTA));

    todasLasMejoras.append(Mejora(Mejora::ARMA, "Arco Multidireccional",
                                  "Dispara en 4 direcciones simultáneas", Arma::ARCO));

    todasLasMejoras.append(Mejora(Mejora::ARMA, "Aceite Hirviendo",
                                  "Área de daño alrededor del jugador", Arma::ACEITE));
}

QList<Mejora> Nivel1::generarOpcionesMejoras(int cantidad)
{
    QList<Mejora> opciones;

    QList<Mejora> armasNuevas;
    for(const Mejora& mejora : todasLasMejoras) {
        if(!jugador->tieneArma(static_cast<Arma::Tipo>(mejora.getTipoArma()))) {
            armasNuevas.append(mejora);
        }
    }

    if(armasNuevas.size() < cantidad) {
        for(Arma* arma : jugador->getArmas()) {
            if(armasNuevas.size() >= cantidad) break;

            QString nombreMejora = QString("Mejorar %1").arg(arma->getNombre());
            QString descripcion = QString("Aumenta el nivel de %1").arg(arma->getNombre());

            Mejora mejora(Mejora::ARMA, nombreMejora, descripcion, arma->getTipo());
            armasNuevas.append(mejora);
        }
    }

    for (int i = 0; i < armasNuevas.size(); ++i) {
        int j = QRandomGenerator::global()->bounded(armasNuevas.size());
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
    qDebug() << "Aplicando mejora:" << mejora.getNombre();
    mejora.aplicar(jugador);

    AudioManager::getInstance().playLevelUp();
}

void Nivel1::mostrarOpcionesMejoras()
{
    mostrandoMejoras = true;
    opcionSeleccionada = 0;
    resetearTeclas();
    pausarNivel();

    opcionesMejorasActuales = generarOpcionesMejoras(3);

    qDebug() << "🎯 Menú de mejoras activado - Opciones:" << opcionesMejorasActuales.size();
}

void Nivel1::actualizarCamara()
{
    if(!jugador) return;

    QPointF objetivo = jugador->getPosicion() - QPointF(tamanoVista.width()/2, tamanoVista.height()/2);

    posicionCamara += (objetivo - posicionCamara) * 0.05f;

    QRectF limitesMapa = mapa->getLimitesMapa();

    posicionCamara.setX(qMax(limitesMapa.left(), qMin(posicionCamara.x(),
                                                      limitesMapa.right() - tamanoVista.width())));
    posicionCamara.setY(qMax(limitesMapa.top(), qMin(posicionCamara.y(),
                                                     limitesMapa.bottom() - tamanoVista.height())));

    static int debugCounter = 0;
    if(++debugCounter % 60 == 0) {
        qDebug() << " Cámara - Pos:" << posicionCamara
                 << "Jugador:" << jugador->getPosicion()
                 << "Vista:" << getVistaCamara();
    }
}

QRectF Nivel1::getVistaCamara() const
{
    return QRectF(posicionCamara, tamanoVista);
}

bool Nivel1::estaEnVista(const QPointF& posicion) const
{
    QRectF vista = getVistaCamara();
    return vista.contains(posicion);
}

bool Nivel1::estaEnVista(const QRectF& area) const
{
    QRectF vista = getVistaCamara();
    return vista.intersects(area);
}

void Nivel1::actualizarJuego()
{
    // VERIFICACIONES DE SEGURIDAD AL INICIO
    if (!jugador || !mapa) {
        qDebug() << "❌ ERROR: Jugador o mapa es nullptr en actualizarJuego";
        return;
    }

    qint64 tiempoActual = QDateTime::currentMSecsSinceEpoch();
    float deltaTime = tiempoActual - tiempoUltimoFrame;
    tiempoUltimoFrame = tiempoActual;

    if(mostrandoMejoras) {
        return;
    }

    // ACTUALIZAR ARMAS DEL JUGADOR
    if (!jugador->getArmas().isEmpty()) {
        for(Arma* arma : jugador->getArmas()) {
            if (arma) {
                arma->setEnemigosCercanos(enemigos);
            }
        }
    }

    // CONTADOR DE TIEMPO
    static float acumuladorTiempo = 0;
    acumuladorTiempo += deltaTime;
    if(acumuladorTiempo >= 1000) {
        tiempoTranscurrido++;
        acumuladorTiempo = 0;

        if(tiempoTranscurrido % 10 == 0) {
            qDebug() << "⏰ Tiempo:" << tiempoTranscurrido << "/" << tiempoObjetivo
                     << "Oleada:" << numeroOleada << "Enemigos:" << enemigos.size();
        }
    }

    // MOVIMIENTO DEL JUGADOR
    QPointF posicionAnterior = jugador->getPosicion();
    jugador->procesarInput(teclas);

    if (jugador) {
        jugador->actualizar(deltaTime);
    }

    // VERIFICAR COLISIONES DEL JUGADOR CON EL MAPA
    QRectF areaJugador = jugador->getAreaColision();
    if(!verificarColisionMapa(areaJugador)) {
        jugador->setPosicion(posicionAnterior);
    }

    // MANTENER JUGADOR DENTRO DE LOS LÍMITES DEL MAPA
    verificarYCorregirLimitesMapa(jugador);

    // ACTUALIZAR CÁMARA
    actualizarCamara();

    // SONIDO DE MOVIMIENTO MEJORADO
    static int moveSoundCooldown = 0;
    bool isMoving = (teclas[0] || teclas[1] || teclas[2] || teclas[3]);

    if (isMoving && moveSoundCooldown <= 0) {
        AudioManager::getInstance().playPlayerMove();
        moveSoundCooldown = 30; // Cooldown aumentado para no saturar
    }

    if (moveSoundCooldown > 0) {
        moveSoundCooldown--;
    }

    // ACTUALIZAR ENEMIGOS
    for(Enemigo *enemigo : enemigos) {
        if(enemigo && enemigo->estaViva()) {
            QPointF posicionAnteriorEnemigo = enemigo->getPosicion();

            // SEGUIR AL JUGADOR SOLO SI ESTÁ CERCA
            float distanciaAlJugador = QLineF(enemigo->getPosicion(), jugador->getPosicion()).length();
            if(distanciaAlJugador < 2000.0f) {
                enemigo->seguirJugador(jugador->getPosicion());
            }

            // ACTUALIZAR ENEMIGO
            enemigo->actualizar(deltaTime);

            // VERIFICAR COLISIONES DEL ENEMIGO CON EL MAPA
            QRectF areaEnemigo = enemigo->getAreaColision();
            if(!verificarColisionMapa(areaEnemigo)) {
                enemigo->setPosicion(posicionAnteriorEnemigo);
            }

            // MANTENER ENEMIGO DENTRO DE LOS LÍMITES DEL MAPA
            verificarYCorregirLimitesMapa(enemigo);
        }
    }

    // PROCESAR COLISIONES (CON CONTROL DE SONIDOS)
    procesarColisiones();

    // LIMPIAR ENEMIGOS MUERTOS
    limpiarEnemigosMuertos();

    // ACTUALIZAR ANIMACIONES DEL MAPA
    if(mapa) {
        mapa->actualizarAnimaciones(deltaTime);
    }

    // VERIFICAR MEJORAS PENDIENTES
    if(jugador->tieneMejoraPendiente() && !mostrandoMejoras) {
        mostrarOpcionesMejoras();
    }

    // VERIFICAR FIN DEL NIVEL
    if(tiempoTranscurrido >= tiempoObjetivo) {
        timerJuego->stop();
        timerOleadas->stop();
        qDebug() << "Nivel completado! Has sobrevivido" << tiempoObjetivo << "segundos";
        emit levelCompleted();
        return; // Salir temprano para evitar updates innecesarios
    }

    // VERIFICAR GAME OVER
    if(!jugador->estaViva()) {
        timerJuego->stop();
        timerOleadas->stop();
        qDebug() << "Game Over - Has sido derrotado";
        emit gameOver();
        return; // Salir temprano para evitar updates innecesarios
    }

    // ACTUALIZAR INTERFAZ
    update();
}

void Nivel1::verificarYCorregirLimitesMapa(Entidad* entidad)
{
    if(!entidad || !mapa) return;

    QRectF limitesMapa = mapa->getLimitesMapa();
    QPointF posicion = entidad->getPosicion();
    QRectF area = entidad->getAreaColision();

    bool necesitaAjuste = false;
    QPointF nuevaPosicion = posicion;

    if(area.left() < limitesMapa.left()) {
        nuevaPosicion.setX(nuevaPosicion.x() + (limitesMapa.left() - area.left()));
        necesitaAjuste = true;
    }
    if(area.right() > limitesMapa.right()) {
        nuevaPosicion.setX(nuevaPosicion.x() - (area.right() - limitesMapa.right()));
        necesitaAjuste = true;
    }
    if(area.top() < limitesMapa.top()) {
        nuevaPosicion.setY(nuevaPosicion.y() + (limitesMapa.top() - area.top()));
        necesitaAjuste = true;
    }
    if(area.bottom() > limitesMapa.bottom()) {
        nuevaPosicion.setY(nuevaPosicion.y() - (area.bottom() - limitesMapa.bottom()));
        necesitaAjuste = true;
    }

    if(necesitaAjuste) {
        entidad->setPosicion(nuevaPosicion);
    }
}

bool Nivel1::verificarColisionMapa(const QRectF& area) const
{
    if(!mapa) return true;

    float stepX = area.width() / 4;
    float stepY = area.height() / 4;

    for(float x = area.left(); x <= area.right(); x += stepX) {
        for(float y = area.top(); y <= area.bottom(); y += stepY) {
            if(!mapa->esTransitable(x, y)) {
                return false;
            }
        }
    }

    return true;
}

void Nivel1::generarOleada()
{
    int enemigosBase = 3 + (numeroOleada / 2);
    int enemigosExtra = QRandomGenerator::global()->bounded(3);

    if(numeroOleada >= 5) enemigosBase += 2;
    if(numeroOleada >= 10) {
        enemigosBase += 3;
        enemigosExtra = QRandomGenerator::global()->bounded(5);
    }

    qDebug() << "OLEADA" << numeroOleada << "-" << (enemigosBase + enemigosExtra) << "enemigos";

    for(int i = 0; i < enemigosBase + enemigosExtra; i++) {
        generarEnemigo();
    }

    numeroOleada++;
    frecuenciaGeneracion = qMax(1200, frecuenciaGeneracion - 80);
    timerOleadas->setInterval(frecuenciaGeneracion);

}

void Nivel1::generarEnemigo()
{
    int tipoEnemigo;
    int probabilidadFuerte = qMin(15 + (numeroOleada - 1) * 8, 75);

    if(QRandomGenerator::global()->bounded(100) < probabilidadFuerte) {
        tipoEnemigo = 2; // Fuerte
    } else {
        tipoEnemigo = 1; // Débil
    }

    Enemigo *enemigo = new Enemigo(tipoEnemigo);

    QRectF limitesMapa = mapa->getLimitesMapa();
    QRectF vistaActual = getVistaCamara();

    float margenSpawnCercano = 50.0f;
    float margenSpawnLejano = 200.0f;

    QRectF areaNoSpawn = vistaActual.adjusted(-margenSpawnCercano, -margenSpawnCercano,
                                              margenSpawnCercano, margenSpawnCercano);

    QPointF posicion;
    int intentos = 0;
    const int MAX_INTENTOS = 30;

    do {
        bool spawnCercano = QRandomGenerator::global()->bounded(100) < 80;
        float margenUsar = spawnCercano ? margenSpawnCercano : margenSpawnLejano;

        int borde = QRandomGenerator::global()->bounded(4);
        switch(borde) {
        case 0:
            posicion = QPointF(
                vistaActual.left() + QRandomGenerator::global()->bounded(vistaActual.width()),
                vistaActual.top() - margenUsar
                );
            break;
        case 1:
            posicion = QPointF(
                vistaActual.right() + margenUsar,
                vistaActual.top() + QRandomGenerator::global()->bounded(vistaActual.height())
                );
            break;
        case 2:
            posicion = QPointF(
                vistaActual.left() + QRandomGenerator::global()->bounded(vistaActual.width()),
                vistaActual.bottom() + margenUsar
                );
            break;
        case 3:
            posicion = QPointF(
                vistaActual.left() - margenUsar,
                vistaActual.top() + QRandomGenerator::global()->bounded(vistaActual.height())
                );
            break;
        }

        posicion.setX(qMax(limitesMapa.left() + 10.0f, qMin(posicion.x(), limitesMapa.right() - 10.0f)));
        posicion.setY(qMax(limitesMapa.top() + 10.0f, qMin(posicion.y(), limitesMapa.bottom() - 10.0f)));

        intentos++;
    } while ((areaNoSpawn.contains(posicion) || !mapa->esTransitable(posicion.x(), posicion.y())) &&
             intentos < MAX_INTENTOS);

    if (intentos >= MAX_INTENTOS) {
        int esquina = QRandomGenerator::global()->bounded(4);
        switch(esquina) {
        case 0: posicion = vistaActual.topLeft() + QPointF(50, 50); break;
        case 1: posicion = vistaActual.topRight() + QPointF(-50, 50); break;
        case 2: posicion = vistaActual.bottomLeft() + QPointF(50, -50); break;
        case 3: posicion = vistaActual.bottomRight() + QPointF(-50, -50); break;
        }

        posicion.setX(qMax(limitesMapa.left() + 10.0f, qMin(posicion.x(), limitesMapa.right() - 10.0f)));
        posicion.setY(qMax(limitesMapa.top() + 10.0f, qMin(posicion.y(), limitesMapa.bottom() - 10.0f)));

        qDebug() << "⚠️ Enemigo spawn en posición de emergencia:" << posicion;
    }

    enemigo->setPosicion(posicion);
    enemigos.append(enemigo);

    static int enemigoCounter = 0;
    enemigoCounter++;
    if(enemigoCounter % 10 == 0) {
        qDebug() << "👹 Enemigo" << enemigoCounter << "spawn en:" << posicion
                 << "Tipo:" << tipoEnemigo << "Vista:" << vistaActual.topLeft();
    }
}

void Nivel1::procesarColisiones()
{
    if (!jugador || enemigos.isEmpty()) return;

    QHash<Enemigo*, bool> enemigosGolpeadosPorArmas;

    // ELIMINAR la reproducción de sonido de flecha aquí
    // Solo mantener el sonido de golpe si quieres
    static int cooldownGolpe = 0;
    bool sonidoGolpeReproducido = false;

    // VERIFICAR COLISIONES CON ARMAS
    for(Arma* arma : jugador->getArmas()) {
        if (!arma) continue;

        QList<QRectF> areasAtaque = arma->getAreasAtaque();

        for(const QRectF& area : areasAtaque) {
            for(Enemigo* enemigo : enemigos) {
                if(enemigo && enemigo->estaViva() &&
                    !enemigosGolpeadosPorArmas.value(enemigo, false) &&
                    area.intersects(enemigo->getAreaColision())) {

                    // ELIMINADO: Sonido de flecha aquí
                    // SOLO mantener sonido de impacto/golpe

                    // APLICAR DAÑO
                    enemigo->recibirDanio(arma->getDanio());
                    enemigosGolpeadosPorArmas[enemigo] = true;

                    // SONIDO GOLPE - Solo una vez por frame
                    if (!sonidoGolpeReproducido && cooldownGolpe <= 0) {
                        AudioManager::getInstance().playEnemyHit();
                        sonidoGolpeReproducido = true;
                        cooldownGolpe = 3; // Cooldown en frames
                    }

                    // VERIFICAR SI EL ENEMIGO MURIÓ
                    if(!enemigo->estaViva()) {
                        jugador->ganarExperiencia(enemigo->getExperienciaQueDa());
                    }
                }
            }
        }
    }

    // REDUCIR COOLDOWNS
    if (cooldownGolpe > 0) cooldownGolpe--;

    // COLISIONES JUGADOR-ENEMIGO (DAÑO POR CONTACTO)
    for(Enemigo* enemigo : enemigos) {
        if(enemigo && enemigo->estaViva() &&
            jugador->getAreaColision().intersects(enemigo->getAreaColision())) {
            jugador->recibirDanio(0.5f);
        }
    }
}

void Nivel1::procesarSeleccionMejora(int opcion)
{
    if (opcion < 0 || opcion >= opcionesMejorasActuales.size()) return;

    const Mejora& mejoraSeleccionada = opcionesMejorasActuales[opcion];

    qDebug() << "✅ Mejora seleccionada:" << mejoraSeleccionada.getNombre();

    aplicarMejora(mejoraSeleccionada);
    jugador->setMejoraPendiente(false);
    mostrandoMejoras = false;
    opcionesMejorasActuales.clear();

    // Pequeño delay antes de reanudar
    QTimer::singleShot(300, this, &Nivel1::onMejoraSeleccionada);
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

void Nivel1::onMejoraSeleccionada()
{
    resetearTeclas();
    reanudarNivel();
}

void Nivel1::keyPressEvent(QKeyEvent *event)
{
    if (mostrandoMejoras) {
        switch(event->key()) {
        case Qt::Key_A:
        case Qt::Key_Left:
            // Navegar izquierda
            opcionSeleccionada = (opcionSeleccionada - 1 + opcionesMejorasActuales.size()) % opcionesMejorasActuales.size();
            update();
            break;

        case Qt::Key_D:
        case Qt::Key_Right:
            // Navegar derecha
            opcionSeleccionada = (opcionSeleccionada + 1) % opcionesMejorasActuales.size();
            update();
            break;

        case Qt::Key_Space:
        case Qt::Key_Return:
        case Qt::Key_Enter:
            // Seleccionar mejora
            procesarSeleccionMejora(opcionSeleccionada);
            break;

        case Qt::Key_1:
            procesarSeleccionMejora(0);
            break;
        case Qt::Key_2:
            procesarSeleccionMejora(1);
            break;
        case Qt::Key_3:
            procesarSeleccionMejora(2);
            break;
        }
        return;
    }

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

void Nivel1::dibujarEntidadConSprite(QPainter &painter, const QPointF &posicionRelativa, const QString &spriteName, const QSize &displaySize, int frameWidth, int frameHeight, int currentFrame) {

    QPixmap spriteSheet = SpriteManager::getInstance().getSprite(spriteName);

    if(!spriteSheet.isNull()) {
        QRect frameRect(currentFrame * frameWidth, 0, frameWidth, frameHeight);
        QPixmap frame = spriteSheet.copy(frameRect);

        QRectF displayRect(posicionRelativa.x() - displaySize.width()/2,
                           posicionRelativa.y() - displaySize.height()/2,
                           displaySize.width(), displaySize.height());
        painter.drawPixmap(displayRect, frame, frame.rect());
    } else {
        // Fallback simple
        painter.setBrush(QBrush(QColor(255, 100, 100)));
        painter.setPen(QPen(Qt::white, 2));
        painter.drawEllipse(posicionRelativa, displaySize.width()/2, displaySize.height()/2);
    }
}

void Nivel1::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF vistaCamara = getVistaCamara();

    // DIBUJAR EL MAPA
    if(mapa && !mapa->getMapaCompleto().isNull()) {
        mapa->dibujar(painter, vistaCamara);
    } else {
        painter.fillRect(rect(), QBrush(QColor(80, 80, 120)));
    }

    // DEBUG: Dibujar área de colisión del jugador
    QPointF playerPos = jugador->getPosicion();
    QPointF playerPosRelativa = playerPos - vistaCamara.topLeft();
    QRectF areaColision = jugador->getAreaColision();
    QRectF areaColisionRelativa = areaColision.translated(-vistaCamara.topLeft());

    painter.setPen(QPen(Qt::red, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(areaColisionRelativa);
    // === ANIMACIÓN DEL JUGADOR SIMPLIFICADA ===
    static int currentFrame = 0;
    static int animationCounter = 0;

    bool isMoving = (teclas[0] || teclas[1] || teclas[2] || teclas[3]);
    animationCounter++;
    if(animationCounter >= 8) {
        currentFrame = (currentFrame + 1) % 6;
        animationCounter = 0;
    }

    // *** SIMPLIFICADO: Usar método helper para dibujar jugador ***
    QString playerSprite = isMoving ? "player_move" : "player_idle";
    dibujarEntidadConSprite(painter, playerPosRelativa, playerSprite,
                            QSize(80, 80), 192, 192, currentFrame);

    // *** SIMPLIFICADO: Dibujar enemigos con método helper ***
    int enemyFrame = (QDateTime::currentMSecsSinceEpoch() / 150) % 6;
    int enemigosEnVista = 0;

    for(Enemigo *enemigo : enemigos) {
        if(enemigo->estaViva()) {
            QPointF enemyPos = enemigo->getPosicion();
            bool enVista = estaEnVista(enemyPos);

            if(enVista) {
                enemigosEnVista++;
                QPointF enemyPosRelativa = enemyPos - vistaCamara.topLeft();
                int enemyType = enemigo->getTipo();

                QString enemySprite = (enemyType == 1) ? "enemy_weak" : "enemy_strong";
                QSize enemySize = (enemyType == 1) ? QSize(60, 60) : QSize(70, 70);
                int frameSize = (enemyType == 1) ? 320 : 192;

                dibujarEntidadConSprite(painter, enemyPosRelativa, enemySprite,
                                        enemySize, frameSize, frameSize, enemyFrame);

                dibujarBarraVidaEnemigo(painter, enemigo, enemyPosRelativa);
            }
        }
    }

    // DIBUJAR ARMAS
    dibujarArmas(painter);

    // DIBUJAR HUD MEJORADO
    dibujarHUD(painter);

    // NUEVO: Dibujar menú de mejoras si está activo
    if (mostrandoMejoras) {
        dibujarMenuMejoras(painter);
    }

    QPixmap cursor = UIManager::getInstance().getCursor();
    if(!cursor.isNull()) {
        QPoint cursorPos = mapFromGlobal(QCursor::pos());
        painter.drawPixmap(cursorPos.x() - cursor.width()/2,
                           cursorPos.y() - cursor.height()/2,
                           cursor);
    }
}

void Nivel1::dibujarArmas(QPainter &painter)
{
    QRectF vistaCamara = getVistaCamara();

    for(Arma* arma : jugador->getArmas()) {

        QList<Arma::ProyectilSprite> proyectiles = arma->getProyectilesSprites();
        for(const auto& proyectil : proyectiles) {
            if(estaEnVista(proyectil.posicion)) {
                QPointF proyectilRelativo = proyectil.posicion - vistaCamara.topLeft();

                QPixmap sprite = SpriteManager::getInstance().getSprite("projectile_arrow");
                if(!sprite.isNull()) {
                    painter.save();
                    painter.translate(proyectilRelativo);
                    painter.rotate(proyectil.rotacion);

                    QSize displaySize(24, 24);
                    QRectF destRect(-displaySize.width()/2, -displaySize.height()/2,
                                    displaySize.width(), displaySize.height());
                    painter.drawPixmap(destRect, sprite, sprite.rect());

                    painter.restore();
                } else {
                    if(arma->getTipo() == Arma::BALLESTA) {
                        painter.setBrush(QBrush(QColor(139, 69, 19)));
                    } else {
                        painter.setBrush(QBrush(QColor(160, 82, 45)));
                    }
                    painter.drawEllipse(proyectilRelativo, 6, 6);
                }
            }
        }

        QList<Arma::AreaAtaqueSprite> areas = arma->getAreasAtaqueSprites();
        for(const auto& areaSprite : areas) {
            if(estaEnVista(areaSprite.area)) {
                QRectF areaRelativa = areaSprite.area.translated(-vistaCamara.topLeft());

                if(arma->getTipo() == Arma::ACEITE) {
                    dibujarAtaqueAceite(painter, arma, areaSprite, areaRelativa);
                }
            }
        }
    }
}

void Nivel1::dibujarAtaqueAceite(QPainter &painter, Arma* arma, const Arma::AreaAtaqueSprite& areaSprite, const QRectF& areaRelativa)
{
    QPixmap spriteFrame = SpriteManager::getInstance().getSpriteFrame(
        areaSprite.spriteName,
        areaSprite.frameActual % areaSprite.totalFrames
        );

    if(!spriteFrame.isNull()) {
        painter.save();

        QPointF centro = areaRelativa.center();
        painter.translate(centro);

        // Para el aceite, usar el tamaño del área pero mantener la relación de aspecto
        float radio = areaRelativa.width() / 2;
        QRectF destRect(-radio, -radio, radio * 2, radio * 2);

        painter.drawPixmap(destRect, spriteFrame, spriteFrame.rect());
        painter.restore();

    } else {
        // Fallback: dibujo geométrico
        QColor colorArma = arma->getColor();
        painter.setBrush(QBrush(colorArma, Qt::DiagCrossPattern));
        painter.setPen(QPen(colorArma.darker(), 2));
        painter.drawEllipse(areaRelativa);
    }
}

void Nivel1::dibujarHUD(QPainter &painter)
{
    UIManager& ui = UIManager::getInstance();

    const int hudX = 10;
    const int hudY = 10;
    const int panelWidth = 192;
    const int panelHeight = 192;
    const int ribbonHeight = 64;

    if(!ui.getHudPanel().isNull()) {
        painter.drawPixmap(hudX, hudY, panelWidth, panelHeight, ui.getHudPanel());
    } else {
        painter.fillRect(hudX, hudY, panelWidth, panelHeight, QColor(0, 0, 0, 180));
    }

    int ribbonY = hudY + panelHeight + 5;

    int panelCenterX = hudX + panelWidth / 2;
    int textY = hudY + 35;
    float scale = 1.0f;
    int lineHeight = ui.getTextHeight(scale) + 5;

    QString vidaText = QString("VIDA: %1").arg(jugador->getVida());
    int vidaWidth = ui.getTextWidth(vidaText, scale);
    ui.drawText(painter, vidaText, panelCenterX - vidaWidth/2, textY, scale);
    textY += lineHeight;

    QString nivelText = QString("NIVEL: %1").arg(jugador->getNivel());
    int nivelWidth = ui.getTextWidth(nivelText, scale);
    ui.drawText(painter, nivelText, panelCenterX - nivelWidth/2, textY, scale);
    textY += lineHeight;

    QString expText = QString("EXP: %1/%2").arg(jugador->getExperiencia()).arg(jugador->getExperienciaParaSiguienteNivel());
    int expWidth = ui.getTextWidth(expText, scale);
    ui.drawText(painter, expText, panelCenterX - expWidth/2, textY, scale);
    textY += lineHeight + 10;

    QString tiempoText = QString("TIEMPO: %1/%2").arg(tiempoTranscurrido).arg(tiempoObjetivo);
    int tiempoWidth = ui.getTextWidth(tiempoText, scale);
    ui.drawText(painter, tiempoText, panelCenterX - tiempoWidth/2, textY, scale);
    textY += lineHeight;

    QString oleadaText = QString("OLEADA: %1").arg(numeroOleada);
    int oleadaWidth = ui.getTextWidth(oleadaText, scale);
    ui.drawText(painter, oleadaText, panelCenterX - oleadaWidth/2, textY, scale);
    textY += lineHeight;

    QString enemigosText = QString("ENEMIGOS: %1").arg(enemigos.size());
    int enemigosWidth = ui.getTextWidth(enemigosText, scale);
    ui.drawText(painter, enemigosText, panelCenterX - enemigosWidth/2, textY, scale);


    if(jugador->tieneMejoraPendiente() && !mostrandoMejoras) {
        int alertY = ribbonY + ribbonHeight + 15;
        QString alertText = "!MEJORA DISPONIBLE!";
        int alertWidth = ui.getTextWidth(alertText, 1.0f);

        if(!ui.getRibbonRed().isNull()) {
            int ribbonWidth = alertWidth + 30;
            painter.drawPixmap(panelCenterX - ribbonWidth/2, alertY - 20, ribbonWidth, ribbonHeight, ui.getRibbonRed());
        }

        ui.drawText(painter, alertText, panelCenterX - alertWidth/2, alertY + 10, 1.0f);
    }
}

void Nivel1::dibujarBarraVidaEnemigo(QPainter &painter, Enemigo *enemigo, const QPointF &posicionRelativa)
{
    float vidaPorcentaje = enemigo->getVida() / (enemigo->getTipo() == 1 ? 25.0f : 70.0f);

    // Barra de vida sobre el enemigo (usando posición relativa)
    QRectF barraFondo(posicionRelativa.x() - 15, posicionRelativa.y() - 30, 30, 4);
    QRectF barraVida(posicionRelativa.x() - 15, posicionRelativa.y() - 30, 30 * vidaPorcentaje, 4);

    painter.setBrush(QBrush(QColor(50, 50, 50)));
    painter.setPen(Qt::NoPen);
    painter.drawRect(barraFondo);

    // Color según tipo
    if(enemigo->getTipo() == 1) {
        painter.setBrush(QBrush(QColor(255, 100, 100)));
    } else {
        painter.setBrush(QBrush(QColor(255, 50, 50)));
    }
    painter.drawRect(barraVida);
}

void Nivel1::dibujarMenuMejoras(QPainter &painter)
{
    if (!mostrandoMejoras || opcionesMejorasActuales.isEmpty()) return;

    UIManager& ui = UIManager::getInstance();

    painter.fillRect(rect(), QColor(0, 0, 0, 200));

    QString titulo = "MEJORA";
    int tituloWidth = ui.getTextWidth(titulo, 1.5f);
    int tituloX = rect().center().x() - tituloWidth/2;
    int tituloY = 80;

    if(!ui.getRibbonBlue().isNull()) {
        int ribbonWidth = tituloWidth + 40;
        painter.drawPixmap(tituloX - 20, tituloY - 30, ribbonWidth, 64, ui.getRibbonBlue());
    }
    ui.drawText(painter, titulo, tituloX, tituloY, 1.5f);

    QString instrucciones = "A/D PARA NAVEGAR   |   ESPACIO PARA SELECCIONAR";
    int instWidth = ui.getTextWidth(instrucciones, 0.9f);
    int instX = rect().center().x() - instWidth/2;
    int instY = tituloY + 50;

    ui.drawText(painter, instrucciones, instX, instY, 0.9f);

    int optionWidth = 450;
    int optionHeight = 100;
    int startY = instY + 50;

    for (int i = 0; i < opcionesMejorasActuales.size(); ++i) {
        const Mejora& mejora = opcionesMejorasActuales[i];

        QRect optionRect(rect().center().x() - optionWidth/2,
                         startY + i * optionHeight,
                         optionWidth,
                         optionHeight - 15);

        if (i == opcionSeleccionada) {
            if(!ui.getRibbonRed().isNull()) {
                painter.drawPixmap(optionRect, ui.getRibbonRed(), ui.getRibbonRed().rect());
            } else {
                painter.fillRect(optionRect, QColor(100, 100, 200, 200));
            }
            painter.setPen(QPen(Qt::yellow, 2));
        } else {
            if(!ui.getRibbonBlue().isNull()) {
                painter.drawPixmap(optionRect, ui.getRibbonBlue(), ui.getRibbonBlue().rect());
            } else {
                painter.fillRect(optionRect, QColor(60, 60, 100, 200));
            }
            painter.setPen(QPen(Qt::white, 1));
        }

        int optionCenterX = optionRect.center().x();
        int textY = optionRect.top() + 30;

        QString textoOpcion = QString("%1. %2").arg(i + 1).arg(mejora.getNombre());
        int textoWidth = ui.getTextWidth(textoOpcion, i == opcionSeleccionada ? 1.1f : 1.0f);
        ui.drawText(painter, textoOpcion, optionCenterX - textoWidth/2, textY,
                    i == opcionSeleccionada ? 1.1f : 1.0f);

        QString descripcion = mejora.getDescripcion();
        int descY = textY + 25;
        int descWidth = ui.getTextWidth(descripcion, 0.8f);

        if(descWidth <= optionWidth - 40) {
            ui.drawText(painter, descripcion, optionCenterX - descWidth/2, descY, 0.8f);
        } else {

            int maxChars = (optionWidth - 40) / ui.getTextWidth("A", 0.8f);
            int breakPoint = descripcion.lastIndexOf(' ', maxChars);
            if(breakPoint == -1) breakPoint = maxChars;

            QString linea1 = descripcion.left(breakPoint).trimmed();
            QString linea2 = descripcion.mid(breakPoint + 1).trimmed();

            int linea1Width = ui.getTextWidth(linea1, 0.8f);
            int linea2Width = ui.getTextWidth(linea2, 0.8f);

            ui.drawText(painter, linea1, optionCenterX - linea1Width/2, descY, 0.8f);
            ui.drawText(painter, linea2, optionCenterX - linea2Width/2, descY + 15, 0.8f);
        }
    }

    int confY = startY + opcionesMejorasActuales.size() * optionHeight + 20;
    QString confirmacion = QString("ESPACIO\n PARA\n CONFIRMAR").arg(opcionSeleccionada + 1);
    int confWidth = ui.getTextWidth(confirmacion, 0.9f);

    if(!ui.getRibbonGreen().isNull()) {
        int ribbonWidth = confWidth + 60;
        int ribbonHeight = 40;
        int textOffsetY = 5;

        painter.drawPixmap(rect().center().x() - ribbonWidth/2, confY - 10, ribbonWidth, ribbonHeight, ui.getRibbonGreen());
        ui.drawText(painter, confirmacion, rect().center().x() - confWidth/2, confY + textOffsetY, 0.9f);
    } else {
        ui.drawText(painter, confirmacion, rect().center().x() - confWidth/2, confY + 10, 0.9f);
    }
}
