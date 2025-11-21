#include "nivel1.h"
#include "spritemanager.h"
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

    SpriteManager::getInstance().preloadGameSprites();

    posicionCamara = QPointF(0, 0);
    margenSpawnExterior = 120.0f;
    margenSpawnInterior = 180.0f;

    mapa = new Mapa(this);
    inicializarMapaGrande();

    jugador = new JugadorNivel1();
    jugador->setPosicion(mapa->getPosicionInicioJugador());

    QPointF posicionInicial = jugador->getPosicion();
    posicionCamara = posicionInicial - QPointF(tamanoVista.width()/2, tamanoVista.height()/2);

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
    qint64 tiempoActual = QDateTime::currentMSecsSinceEpoch();
    float deltaTime = tiempoActual - tiempoUltimoFrame;
    tiempoUltimoFrame = tiempoActual;

    if(mostrandoMejoras) {
        return;
    }

    for(Arma* arma : jugador->getArmas()) {
        arma->setEnemigosCercanos(enemigos);
    }

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

    QPointF posicionAnterior = jugador->getPosicion();
    jugador->procesarInput(teclas);
    jugador->actualizar(deltaTime);

    QRectF areaJugador = jugador->getAreaColision();
    if(!verificarColisionMapa(areaJugador)) {
        jugador->setPosicion(posicionAnterior);
    }

    verificarYCorregirLimitesMapa(jugador);

    actualizarCamara();

    for(Enemigo *enemigo : enemigos) {
        if(enemigo->estaViva()) {
            QPointF posicionAnteriorEnemigo = enemigo->getPosicion();

            float distanciaAlJugador = QLineF(enemigo->getPosicion(), jugador->getPosicion()).length();
            if(distanciaAlJugador < 2000.0f) {
                enemigo->seguirJugador(jugador->getPosicion());
            }

            enemigo->actualizar(deltaTime);

            QRectF areaEnemigo = enemigo->getAreaColision();
            if(!verificarColisionMapa(areaEnemigo)) {
                enemigo->setPosicion(posicionAnteriorEnemigo);
            }

            verificarYCorregirLimitesMapa(enemigo);
        }
    }

    procesarColisiones();
    limpiarEnemigosMuertos();

    if(mapa) {
        mapa->actualizarAnimaciones(deltaTime);
    }

    if(jugador->tieneMejoraPendiente() && !mostrandoMejoras) {
        mostrarOpcionesMejoras();
    }

    if(tiempoTranscurrido >= tiempoObjetivo) {
        timerJuego->stop();
        timerOleadas->stop();
        qDebug() << "Nivel completado! Has sobrevivido" << tiempoObjetivo << "segundos";
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
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 10)); // Fuente un poco más pequeña

    // Información básica - posiciones ajustadas
    painter.drawText(10, 15, QString("Vida: %1").arg(jugador->getVida()));
    painter.drawText(10, 30, QString("Nivel: %1").arg(jugador->getNivel()));

    // Barra de experiencia más pequeña
    int expActual = jugador->getExperiencia();
    int expRequerida = jugador->getExperienciaParaSiguienteNivel();
    float porcentajeEXP = (float)expActual / expRequerida;

    QRectF barraEXPFondo(10, 40, 150, 6); // Más estrecha
    QRectF barraEXP(10, 40, 150 * porcentajeEXP, 6);

    painter.setBrush(QBrush(QColor(50, 50, 50)));
    painter.setPen(Qt::NoPen);
    painter.drawRect(barraEXPFondo);

    painter.setBrush(QBrush(QColor(0, 200, 255)));
    painter.drawRect(barraEXP);

    painter.setPen(Qt::white);
    painter.drawText(15, 48, QString("EXP: %1/%2").arg(expActual).arg(expRequerida));

    // Tiempo y oleadas
    painter.drawText(10, 65, QString("Tiempo: %1/%2").arg(tiempoTranscurrido).arg(tiempoObjetivo));
    painter.drawText(10, 80, QString("Oleada: %1").arg(numeroOleada));
    painter.drawText(10, 95, QString("Enemigos: %1").arg(enemigos.size()));

    // Información de siguiente oleada
    painter.drawText(10, 110, QString("Siguiente: %1s").arg(frecuenciaGeneracion / 1000));

    // Armás activas - posición ajustada
    int yPos = 125;
    painter.drawText(10, yPos, "Armas Activas:");
    yPos += 15;

    for(Arma* arma : jugador->getArmas()) {
        painter.drawText(20, yPos, QString("- %1").arg(arma->getNombre()));
        yPos += 12; // Menos espacio entre líneas
    }

    // Controles - posición ajustada
    painter.drawText(10, 700, "Controles: WASD - Movimiento"); // Más arriba
    painter.drawText(10, 715, "P - Pausa, R - Reanudar");

    if(jugador->tieneMejoraPendiente() && !mostrandoMejoras) {
        painter.setPen(Qt::yellow);
        painter.setFont(QFont("Arial", 12, QFont::Bold));
        painter.drawText(rect().center().x() - 100, 30, "¡MEJORA DISPONIBLE! (Sube de nivel)");
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

    // Fondo semitransparente
    painter.fillRect(rect(), QColor(0, 0, 0, 180));

    // Título
    painter.setPen(Qt::yellow);
    painter.setFont(QFont("Arial", 24, QFont::Bold));
    painter.drawText(rect().center().x() - 150, 100, "¡NUEVA MEJORA DISPONIBLE!");

    // Instrucciones
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 16));
    painter.drawText(rect().center().x() - 120, 140, "Usa A/D para navegar, ESPACIO para seleccionar");

    // Dibujar opciones
    int startY = 200;
    int optionHeight = 120;
    int optionWidth = 600;

    for (int i = 0; i < opcionesMejorasActuales.size(); ++i) {
        const Mejora& mejora = opcionesMejorasActuales[i];

        // Fondo de la opción
        QRect optionRect(rect().center().x() - optionWidth/2,
                         startY + i * optionHeight,
                         optionWidth,
                         optionHeight - 20);

        // Color según si está seleccionada
        if (i == opcionSeleccionada) {
            painter.setBrush(QBrush(QColor(100, 100, 200, 200)));
            painter.setPen(QPen(Qt::yellow, 3));
        } else {
            painter.setBrush(QBrush(QColor(60, 60, 100, 200)));
            painter.setPen(QPen(Qt::white, 1));
        }

        painter.drawRoundedRect(optionRect, 15, 15);

        // Texto de la mejora
        painter.setPen(i == opcionSeleccionada ? Qt::yellow : Qt::white);
        painter.setFont(QFont("Arial", 16, i == opcionSeleccionada ? QFont::Bold : QFont::Normal));

        // Nombre
        painter.drawText(optionRect.left() + 20, optionRect.top() + 30,
                         QString("%1. %2").arg(i + 1).arg(mejora.getNombre()));

        // Descripción
        painter.setFont(QFont("Arial", 12));
        painter.setPen(Qt::lightGray);
        painter.drawText(optionRect.left() + 40, optionRect.top() + 60, mejora.getDescripcion());

        // Indicador de selección
        if (i == opcionSeleccionada) {
            painter.setPen(QPen(Qt::yellow, 2));
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(optionRect.adjusted(2, 2, -2, -2));

            // Flecha selección
            painter.drawText(optionRect.left() + 10, optionRect.top() + 35, "➤");
        }
    }

    // Información adicional
    painter.setPen(Qt::cyan);
    painter.setFont(QFont("Arial", 14));
    painter.drawText(rect().center().x() - 100, startY + opcionesMejorasActuales.size() * optionHeight + 40,
                     QString("Seleccionada: %1 - Presiona ESPACIO para confirmar").arg(opcionSeleccionada + 1));
}
