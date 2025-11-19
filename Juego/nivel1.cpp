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
    // TAMAÑO FIJO 1024x768
    setFixedSize(1024, 768);
    tamanoVista = QSize(1024, 768);

    qDebug() << "🎮 Nivel1 - Vista 1024x768";

    setFocusPolicy(Qt::StrongFocus);

    SpriteManager::getInstance().preloadGameSprites();

    // INICIALIZAR CÁMARA
    posicionCamara = QPointF(0, 0);
    suavizadoCamara = 0.08f;
    margenSpawnExterior = 120.0f;  // Ajustado para vista más pequeña
    margenSpawnInterior = 180.0f;  // Ajustado para vista más pequeña

    // Inicializar mapa GRANDE
    mapa = new Mapa(this);
    inicializarMapaGrande();

    // Inicializar jugador
    jugador = new JugadorNivel1();
    jugador->setPosicion(mapa->getPosicionInicioJugador());

    // Inicializar cámara en posición del jugador
    QPointF posicionInicial = jugador->getPosicion();
    posicionCamara = posicionInicial - QPointF(tamanoVista.width()/2, tamanoVista.height()/2);

    qDebug() << "🎥 Cámara inicializada";

    // Inicializar timers
    timerJuego = new QTimer(this);
    timerOleadas = new QTimer(this);
    tiempoUltimoFrame = QDateTime::currentMSecsSinceEpoch();

    connect(timerJuego, &QTimer::timeout, this, &Nivel1::actualizarJuego);
    connect(timerOleadas, &QTimer::timeout, this, &Nivel1::generarOleada);

    // INICIALIZAR VARIABLES
    for(int i = 0; i < 4; i++) {
        teclas[i] = false;
    }

    tiempoTranscurrido = 0;
    tiempoObjetivo = 120;
    numeroOleada = 1;
    enemigosPorOleada = 4;  // Un poco más de enemigos para vista más pequeña
    frecuenciaGeneracion = 3500; // Oleadas más frecuentes

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

void Nivel1::inicializarMapaGrande()
{
    // MAPA 5x MÁS GRANDE QUE 1024x768
    QSize tamanoMapa(1024 * 5, 768 * 5); // 5120 x 3840 pixels

    qDebug() << "🗺️ Creando mapa 5120x3840 (5x 1024x768)";

    mapa->crearMapaGrande(tamanoMapa);

    qDebug() << "✅ MAPA 5x LISTO - Área enorme para explorar!";
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

void Nivel1::actualizarCamara()
{
    if(!jugador) return;

    QPointF objetivo = jugador->getPosicion() - QPointF(tamanoVista.width()/2, tamanoVista.height()/2);

    float factorSuavizado = 0.1f;
    QPointF camaraAnterior = posicionCamara;
    posicionCamara = posicionCamara + (objetivo - posicionCamara) * factorSuavizado;

    QRectF limitesMapa = mapa->getLimitesMapa();

    bool limitada = false;
    if(posicionCamara.x() < limitesMapa.left()) {
        posicionCamara.setX(limitesMapa.left());
        limitada = true;
    }
    if(posicionCamara.y() < limitesMapa.top()) {
        posicionCamara.setY(limitesMapa.top());
        limitada = true;
    }
    if(posicionCamara.x() + tamanoVista.width() > limitesMapa.right()) {
        posicionCamara.setX(limitesMapa.right() - tamanoVista.width());
        limitada = true;
    }
    if(posicionCamara.y() + tamanoVista.height() > limitesMapa.bottom()) {
        posicionCamara.setY(limitesMapa.bottom() - tamanoVista.height());
        limitada = true;
    }

    static int debugCounter = 0;
    debugCounter++;
    if(debugCounter % 120 == 0) {
        qDebug() << "🎥 Cámara - Jugador:" << jugador->getPosicion()
                 << "Actual:" << posicionCamara
                 << "Limitada:" << limitada;
    }
}

QRectF Nivel1::getVistaCamara() const
{
    return QRectF(posicionCamara, tamanoVista);
}

QPointF Nivel1::calcularPosicionCamara() const
{
    return posicionCamara;
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
    // Calcular deltaTime
    qint64 tiempoActual = QDateTime::currentMSecsSinceEpoch();
    float deltaTime = tiempoActual - tiempoUltimoFrame;
    tiempoUltimoFrame = tiempoActual;

    if(mostrandoMejoras) {
        return;
    }

    // Actualizar tiempo del juego - DEBUG REDUCIDO
    static float acumuladorTiempo = 0;
    acumuladorTiempo += deltaTime;
    if(acumuladorTiempo >= 1000) {
        tiempoTranscurrido++;
        acumuladorTiempo = 0;

        // SOLO mostrar tiempo cada 10 segundos
        if(tiempoTranscurrido % 10 == 0) {
            qDebug() << "⏰ Tiempo:" << tiempoTranscurrido << "/" << tiempoObjetivo
                     << "Oleada:" << numeroOleada << "Enemigos:" << enemigos.size();
        }
    }

    // *** NUEVO: GUARDAR POSICIÓN ANTERIOR PARA COLISIONES ***
    QPointF posicionAnterior = jugador->getPosicion();

    // Actualizar jugador
    jugador->procesarInput(teclas);
    jugador->actualizar(deltaTime);

    // *** VERIFICAR COLISIONES CON MAPA PRIMERO ***
    QRectF areaJugador = jugador->getAreaColision();
    if(!verificarColisionMapa(areaJugador)) {
        // Hay colisión, revertir movimiento
        jugador->setPosicion(posicionAnterior);
        // ELIMINADO: Debug de colisión constante
    }

    // *** SOLO LIMITAR SI REALMENTE SE SALIÓ DEL MAPA ***
    QRectF limitesMapa = mapa->getLimitesMapa();
    QPointF posActual = jugador->getPosicion();
    QRectF areaActual = jugador->getAreaColision();

    bool necesitaAjuste = false;
    QPointF nuevaPosicion = posActual;

    if(areaActual.left() < limitesMapa.left()) {
        nuevaPosicion.setX(nuevaPosicion.x() + (limitesMapa.left() - areaActual.left()));
        necesitaAjuste = true;
    }
    if(areaActual.right() > limitesMapa.right()) {
        nuevaPosicion.setX(nuevaPosicion.x() - (areaActual.right() - limitesMapa.right()));
        necesitaAjuste = true;
    }
    if(areaActual.top() < limitesMapa.top()) {
        nuevaPosicion.setY(nuevaPosicion.y() + (limitesMapa.top() - areaActual.top()));
        necesitaAjuste = true;
    }
    if(areaActual.bottom() > limitesMapa.bottom()) {
        nuevaPosicion.setY(nuevaPosicion.y() - (areaActual.bottom() - limitesMapa.bottom()));
        necesitaAjuste = true;
    }

    if(necesitaAjuste) {
        jugador->setPosicion(nuevaPosicion);
        // ELIMINADO: Debug de ajuste constante
    }

    // *** ACTUALIZAR CÁMARA DESPUÉS DE TODOS LOS AJUSTES ***
    actualizarCamara();

    // Actualizar enemigos
    for(Enemigo *enemigo : enemigos) {
        QPointF posicionAnteriorEnemigo = enemigo->getPosicion();
        enemigo->seguirJugador(jugador->getPosicion());
        enemigo->actualizar(deltaTime);

        // Verificar colisiones de enemigos con mapa
        QRectF areaEnemigo = enemigo->getAreaColision();
        if(!verificarColisionMapa(areaEnemigo)) {
            enemigo->setPosicion(posicionAnteriorEnemigo);
        }

        // Limitar enemigos al mapa (solo si realmente se salen)
        QPointF posEnemigo = enemigo->getPosicion();
        QRectF areaEnemigoActual = enemigo->getAreaColision();

        if(areaEnemigoActual.left() < limitesMapa.left())
            posEnemigo.setX(posEnemigo.x() + (limitesMapa.left() - areaEnemigoActual.left()));
        if(areaEnemigoActual.right() > limitesMapa.right())
            posEnemigo.setX(posEnemigo.x() - (areaEnemigoActual.right() - limitesMapa.right()));
        if(areaEnemigoActual.top() < limitesMapa.top())
            posEnemigo.setY(posEnemigo.y() + (limitesMapa.top() - areaEnemigoActual.top()));
        if(areaEnemigoActual.bottom() > limitesMapa.bottom())
            posEnemigo.setY(posEnemigo.y() - (areaEnemigoActual.bottom() - limitesMapa.bottom()));

        enemigo->setPosicion(posEnemigo);
    }

    // Procesar colisiones entre armas y enemigos
    procesarColisiones();
    limpiarEnemigosMuertos();

    // Mostrar mejoras si hay pendiente
    if(jugador->tieneMejoraPendiente() && !mostrandoMejoras) {
        mostrarOpcionesMejoras();
    }

    // Verificar victoria/derrota
    if(tiempoTranscurrido >= tiempoObjetivo) {
        timerJuego->stop();
        timerOleadas->stop();
        qDebug() << "🎉 ¡Nivel completado! Has sobrevivido" << tiempoObjetivo << "segundos";
        emit levelCompleted();
    }

    if(!jugador->estaViva()) {
        timerJuego->stop();
        timerOleadas->stop();
        qDebug() << "💀 Game Over - Has sido derrotado";
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

    return true; // No hay colisión
}

void Nivel1::generarOleada()
{
    // SISTEMA DE OLEADAS MÁS AGRESIVO
    int enemigosBase = 3 + (numeroOleada / 2);
    int enemigosExtra = QRandomGenerator::global()->bounded(3);

    // A partir de la oleada 5, garantizar más enemigos
    if(numeroOleada >= 5) {
        enemigosBase += 2;
    }

    // A partir de la oleada 10, enjambres más grandes
    if(numeroOleada >= 10) {
        enemigosBase += 3;
        enemigosExtra = QRandomGenerator::global()->bounded(5);
    }

    qDebug() << "🔥 OLEADA" << numeroOleada << "- Generando"
             << (enemigosBase + enemigosExtra) << "enemigos";

    for(int i = 0; i < enemigosBase + enemigosExtra; i++) {
        generarEnemigo();
    }

    numeroOleada++;

    // PROGRESIÓN MÁS AGRESIVA EN FRECUENCIA
    frecuenciaGeneracion = qMax(1200, frecuenciaGeneracion - 80);
    timerOleadas->setInterval(frecuenciaGeneracion);

    qDebug() << "⏱️  Siguiente oleada en:" << frecuenciaGeneracion << "ms";
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

    QRectF areaNoSpawn = vistaActual.adjusted(-margenSpawnInterior, -margenSpawnInterior,
                                              margenSpawnInterior, margenSpawnInterior);

    QPointF posicion;
    int intentos = 0;
    const int MAX_INTENTOS = 20;

    do {
        int borde = QRandomGenerator::global()->bounded(4);
        switch(borde) {
        case 0:
            posicion = QPointF(limitesMapa.left() + QRandomGenerator::global()->bounded(limitesMapa.width()),
                               limitesMapa.top() - margenSpawnExterior);
            break;
        case 1:
            posicion = QPointF(limitesMapa.right() + margenSpawnExterior,
                               limitesMapa.top() + QRandomGenerator::global()->bounded(limitesMapa.height()));
            break;
        case 2:
            posicion = QPointF(limitesMapa.left() + QRandomGenerator::global()->bounded(limitesMapa.width()),
                               limitesMapa.bottom() + margenSpawnExterior);
            break;
        case 3:
            posicion = QPointF(limitesMapa.left() - margenSpawnExterior,
                               limitesMapa.top() + QRandomGenerator::global()->bounded(limitesMapa.height()));
            break;
        }
        intentos++;
    } while (areaNoSpawn.contains(posicion) && intentos < MAX_INTENTOS);

    enemigo->setPosicion(posicion);
    enemigos.append(enemigo);

    static int enemigoCounter = 0;
    enemigoCounter++;
    if(enemigoCounter % 5 == 0) {
        qDebug() << "👹 Generados" << enemigoCounter << "enemigos - Tipo:" << tipoEnemigo
                 << "Total activos:" << enemigos.size();
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

    QRectF vistaCamara = getVistaCamara();

    static int paintCounter = 0;
    paintCounter++;
    if(paintCounter % 300 == 0) {
        qDebug() << "🎨 Paint #" << paintCounter << "- Vista cámara:" << vistaCamara;
        qDebug() << "   Jugador pos:" << jugador->getPosicion();
        qDebug() << "   Enemigos totales:" << enemigos.size();
    }

    // *** NUEVO: DIBUJAR FONDO DE EMERGENCIA SI EL MAPA FALLA ***
    if(!mapa || mapa->getMapaCompleto().isNull()) {
        painter.fillRect(rect(), QBrush(QColor(80, 80, 120)));
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 16));
        painter.drawText(rect().center() - QPoint(150, 0), "MAPA NO CARGADO");
        qDebug() << "❌ ERROR: Mapa no disponible en paintEvent";
        return;
    }

    // DIBUJAR EL MAPA
    QPixmap mapaCompleto = mapa->getMapaCompleto();
    if(mapaCompleto.isNull()) {
        painter.fillRect(rect(), QBrush(QColor(100, 100, 140)));
        painter.setPen(Qt::yellow);
        painter.drawText(10, 30, "MAPA NULO - Usando fondo de emergencia");
    } else {
        // *** NUEVO: DEBUG DEL DIBUJO DEL MAPA ***
        if(paintCounter % 120 == 0) {
            qDebug() << "🎨 Dibujando mapa - Tamaño:" << mapaCompleto.size();
            qDebug() << "   Vista cámara:" << vistaCamara;
        }

        mapa->dibujar(painter, vistaCamara);
    }

    QPointF playerPos = jugador->getPosicion();
    QPointF playerPosRelativa = playerPos - vistaCamara.topLeft();

    // === ANIMACIÓN DEL JUGADOR ===
    static int currentFrame = 0;
    static int animationCounter = 0;

    bool isMoving = (teclas[0] || teclas[1] || teclas[2] || teclas[3]);
    QPixmap playerSpriteSheet = isMoving ?
                                    SpriteManager::getInstance().getSprite("player_move") :
                                    SpriteManager::getInstance().getSprite("player_idle");

    // DIBUJAR JUGADOR
    if(!playerSpriteSheet.isNull()) {
        int frameWidth = 192;
        int frameHeight = 192;

        animationCounter++;
        if(animationCounter >= 8) {
            currentFrame = (currentFrame + 1) % 6;
            animationCounter = 0;
        }

        QRect frameRect(currentFrame * frameWidth, 0, frameWidth, frameHeight);
        QPixmap frame = playerSpriteSheet.copy(frameRect);

        int displayWidth = 80;
        int displayHeight = 80;

        QRectF displayRect(playerPosRelativa.x() - displayWidth/2,
                           playerPosRelativa.y() - displayHeight/2,
                           displayWidth, displayHeight);
        painter.drawPixmap(displayRect, frame, frame.rect());

    } else {
        // *** NUEVO: JUGADOR DE FALLBACK MÁS VISIBLE ***
        painter.setBrush(QBrush(QColor(0, 150, 255))); // Azul brillante
        painter.setPen(QPen(Qt::white, 3));
        painter.drawEllipse(playerPosRelativa, 30, 30);

        // Indicador de dirección
        painter.drawLine(playerPosRelativa,
                         playerPosRelativa + jugador->getUltimaDireccion() * 40);
    }

    // *** NUEVO: CONTADOR DE ENEMIGOS EN PANTALLA ***
    int enemigosEnVista = 0;

    // DIBUJAR ENEMIGOS
    int enemyFrame = (QDateTime::currentMSecsSinceEpoch() / 150) % 6;

    for(Enemigo *enemigo : enemigos) {
        if(enemigo->estaViva()) {
            QPointF enemyPos = enemigo->getPosicion();
            bool enVista = estaEnVista(enemyPos);

            if(enVista) {
                enemigosEnVista++;
                QPointF enemyPosRelativa = enemyPos - vistaCamara.topLeft();
                int enemyType = enemigo->getTipo();

                QString spriteName = (enemyType == 1) ? "enemy_weak" : "enemy_strong";
                QPixmap enemySpriteSheet = SpriteManager::getInstance().getSprite(spriteName);

                if(!enemySpriteSheet.isNull()) {
                    int frameWidth, frameHeight, displayWidth, displayHeight;

                    if(enemyType == 1) {
                        frameWidth = 320; frameHeight = 320;
                        displayWidth = 60; displayHeight = 60;
                    } else {
                        frameWidth = 192; frameHeight = 192;
                        displayWidth = 70; displayHeight = 70;
                    }

                    QRect frameRect(enemyFrame * frameWidth, 0, frameWidth, frameHeight);
                    QPixmap frame = enemySpriteSheet.copy(frameRect);

                    QRectF displayRect(enemyPosRelativa.x() - displayWidth/2,
                                       enemyPosRelativa.y() - displayHeight/2,
                                       displayWidth, displayHeight);
                    painter.drawPixmap(displayRect, frame, frame.rect());

                } else {
                    // *** NUEVO: ENEMIGOS DE FALLBACK MÁS VISIBLES ***
                    if(enemyType == 1) {
                        painter.setBrush(QBrush(QColor(255, 100, 100))); // Rojo brillante
                        painter.drawEllipse(enemyPosRelativa, 25, 25);
                    } else {
                        painter.setBrush(QBrush(QColor(200, 50, 50)));   // Rojo oscuro
                        painter.drawEllipse(enemyPosRelativa, 35, 35);
                    }
                    painter.setPen(QPen(Qt::white, 2));
                }

                // Dibujar barra de vida
                dibujarBarraVidaEnemigo(painter, enemigo, enemyPosRelativa);
            }
        }
    }

    // DIBUJAR ARMAS
    dibujarArmas(painter);

    // DIBUJAR HUD MEJORADO
    dibujarHUD(painter);

    // *** NUEVO: INFO DE DEBUG MEJORADA ***
    painter.setPen(Qt::cyan);
    painter.setFont(QFont("Arial", 12));
    painter.drawText(10, 280, QString("Cámara: %1, %2").arg(vistaCamara.x()).arg(vistaCamara.y()));
    painter.drawText(10, 300, QString("Jugador: %1, %2").arg(playerPos.x()).arg(playerPos.y()));
    painter.drawText(10, 320, QString("Enemigos: %1/%2 en vista").arg(enemigosEnVista).arg(enemigos.size()));
    painter.drawText(10, 340, QString("Mapa: %1x%2").arg(mapa->getTamanoMapa().width()).arg(mapa->getTamanoMapa().height()));
}

void Nivel1::dibujarArmas(QPainter &painter)
{
    // OBTENER VISTA DE CÁMARA ACTUAL
    QRectF vistaCamara = getVistaCamara();

    for(Arma* arma : jugador->getArmas()) {

        // 1. DIBUJAR PROYECTILES (Ballesta y Arco)
        QList<Arma::ProyectilSprite> proyectiles = arma->getProyectilesSprites();
        for(const auto& proyectil : proyectiles) {
            // Verificar si el proyectil está en vista
            if(estaEnVista(proyectil.posicion)) {
                QPointF proyectilRelativo = proyectil.posicion - vistaCamara.topLeft();

                QPixmap sprite = SpriteManager::getInstance().getSprite("projectile_arrow");
                if(!sprite.isNull()) {
                    painter.save();
                    painter.translate(proyectilRelativo);
                    painter.rotate(proyectil.rotacion);

                    // Tamaño del proyectil
                    QSize displaySize(24, 24);
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
                    painter.drawEllipse(proyectilRelativo, 6, 6);
                }
            }
        }

        // 2. DIBUJAR ATAQUES DE ÁREA (Espada y Aceite)
        QList<Arma::AreaAtaqueSprite> areas = arma->getAreasAtaqueSprites();
        for(const auto& areaSprite : areas) {
            // Verificar si el área de ataque está en vista
            if(estaEnVista(areaSprite.area)) {
                // Convertir área a coordenadas relativas
                QRectF areaRelativa = areaSprite.area.translated(-vistaCamara.topLeft());

                if(arma->getTipo() == Arma::ESPADA) {
                    dibujarAtaqueEspada(painter, arma, areaSprite, areaRelativa);
                } else if(arma->getTipo() == Arma::ACEITE) {
                    dibujarAtaqueAceite(painter, arma, areaSprite, areaRelativa);
                }
            }
        }
    }
}

void Nivel1::dibujarAtaqueEspada(QPainter &painter, Arma* arma, const Arma::AreaAtaqueSprite& areaSprite, const QRectF& areaRelativa)
{
    QPixmap spriteFrame = SpriteManager::getInstance().getSpriteFrame(
        areaSprite.spriteName,
        areaSprite.frameActual % areaSprite.totalFrames
        );

    if(!spriteFrame.isNull()) {
        painter.save();

        // Posicionar en el centro del área de ataque RELATIVA
        QPointF centro = areaRelativa.center();
        painter.translate(centro);

        // Aplicar rotación según la dirección del ataque
        painter.rotate(areaSprite.rotacion);

        // Escalar el sprite para que coincida con el área de ataque
        QSizeF spriteSize = areaRelativa.size();
        QRectF destRect(-spriteSize.width()/2, -spriteSize.height()/2,
                        spriteSize.width(), spriteSize.height());

        painter.drawPixmap(destRect, spriteFrame, spriteFrame.rect());
        painter.restore();

    } else {
        // Fallback: dibujo geométrico
        QColor colorArma = arma->getColor();
        painter.setBrush(QBrush(colorArma.lighter(150), Qt::Dense4Pattern));
        painter.setPen(QPen(colorArma.darker(130), 2));
        painter.drawRect(areaRelativa);
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

    if(jugador->tieneMejoraPendiente()) {
        painter.setPen(Qt::yellow);
        painter.setFont(QFont("Arial", 12, QFont::Bold)); // Más pequeño
        painter.drawText(200, 20, "¡MEJORA DISPONIBLE! 1, 2, 3");
    }

    if(mostrandoMejoras) {
        painter.fillRect(rect(), QColor(0, 0, 0, 150));
        painter.setPen(Qt::yellow);
        painter.setFont(QFont("Arial", 14, QFont::Bold)); // Más pequeño
        painter.drawText(rect().center() - QPoint(120, 0), "ELIGE MEJORA (1, 2, 3)");
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
