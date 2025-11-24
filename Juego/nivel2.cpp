#include "nivel2.h"
#include "enemigo.h"
#include "audiomanager.h"
#include "spritemanager.h"
#include "uimanager.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTimer>
#include <QDebug>
#include <QtMath>
#include <QRandomGenerator>

Nivel2::Nivel2(QWidget *parent) : NivelBase(parent)
{
    jugador = new JugadorNivel2();

    timerOleadas = new QTimer(this);
    timerRecursos = new QTimer(this);

    timerJuego->setInterval(16); // ~60 FPS

    connect(timerOleadas, &QTimer::timeout, this, &Nivel2::generarOleada);
    connect(timerRecursos, &QTimer::timeout, this, &Nivel2::generarRecursos);

    inicializarRutas();

    tiempoTranscurrido = 0;
    tiempoObjetivo = 180; // 3 minutos
    numeroOleada = 1;
    recursosGenerados = 0;
    modoConstruccion = true;
    torreSeleccionada = nullptr;

    qDebug() << "Nivel 2 inicializado - Sistema de torres defensivas";
}

Nivel2::~Nivel2()
{
    for(Enemigo* enemigo : enemigos) {
        delete enemigo;
    }
    enemigos.clear();

    if (timerOleadas) {
        timerOleadas->stop();
    }
    if (timerRecursos) {
        timerRecursos->stop();
    }

    qDebug() << "Nivel 2 destruido - recursos limpiados";
}

void Nivel2::inicializarRutas()
{
    rutasEnemigas.clear();

    // Ruta 1: Desde la izquierda hacia el castillo
    QList<QPointF> ruta1;
    ruta1.append(QPointF(-50, 200));
    ruta1.append(QPointF(150, 200));
    ruta1.append(QPointF(300, 350));
    ruta1.append(QPointF(512, 384)); // Centro del castillo
    rutasEnemigas.append(ruta1);

    // Ruta 2: Desde la derecha hacia el castillo
    QList<QPointF> ruta2;
    ruta2.append(QPointF(1074, 300));
    ruta2.append(QPointF(874, 300));
    ruta2.append(QPointF(724, 450));
    ruta2.append(QPointF(512, 384)); // Centro del castillo
    rutasEnemigas.append(ruta2);

    // Ruta 3: Desde abajo hacia el castillo
    QList<QPointF> ruta3;
    ruta3.append(QPointF(400, 818));
    ruta3.append(QPointF(400, 618));
    ruta3.append(QPointF(512, 500));
    ruta3.append(QPointF(512, 384)); // Centro del castillo
    rutasEnemigas.append(ruta3);

    // Ruta 4: Desde arriba hacia el castillo
    QList<QPointF> ruta4;
    ruta4.append(QPointF(600, -50));
    ruta4.append(QPointF(600, 150));
    ruta4.append(QPointF(512, 250));
    ruta4.append(QPointF(512, 384)); // Centro del castillo
    rutasEnemigas.append(ruta4);
}

void Nivel2::iniciarNivel()
{
    NivelBase::iniciarNivel();

    AudioManager::getInstance().stopBackgroundMusic();

    timerOleadas->start(10000);
    timerRecursos->start(5000);

    AudioManager::getInstance().playBackgroundMusic();

    QTimer::singleShot(1000, this, &Nivel2::generarOleada);

    qDebug() << "Nivel 2 iniciado - Modo construcción activado, audio configurado";
    update();
}

void Nivel2::pausarNivel()
{
    NivelBase::pausarNivel(); // Llamar al método base
    timerOleadas->stop();
    timerRecursos->stop();

    update();
}

void Nivel2::reanudarNivel()
{
    NivelBase::reanudarNivel(); // Llamar al método base
    timerOleadas->start();
    timerRecursos->start();

    update();
}

void Nivel2::actualizarJuego(float deltaTime)
{
    // Verificar si el juego está pausado verificando si el timer está activo
    if (!timerJuego->isActive()) return;

    // Actualizar tiempo transcurrido
    tiempoTranscurrido += deltaTime / 1000.0f; // Convertir a segundos

    // Verificar victoria
    if (tiempoTranscurrido >= tiempoObjetivo) {
        emit levelCompleted();
        pausarNivel();
        return;
    }

    // Actualizar jugador (gestión de torres)
    jugador->actualizar(deltaTime);

    // Actualizar enemigos - seguir al castillo
    for (Enemigo* enemigo : enemigos) {
        if (enemigo->estaViva()) {
            // Hacer que el enemigo siga al castillo (posición fija)
            QPointF posicionCastillo(512, 384);
            enemigo->seguirJugador(posicionCastillo);
            enemigo->actualizar(deltaTime);
        }
    }

    // Actualizar torres y procesar objetivos
    actualizarTorres();

    // Procesar colisiones
    procesarColisiones();

    // Limpiar enemigos muertos
    limpiarEnemigosMuertos();

    // Verificar derrota (si el castillo es destruido)
    if (jugador->getVida() <= 0) {
        emit gameOver();
        pausarNivel();
        return;
    }

    update();
}

void Nivel2::actualizarTorres()
{
    JugadorNivel2* jugadorN2 = static_cast<JugadorNivel2*>(jugador);

    // Para cada torre, encontrar enemigos en su rango
    for (Torre* torre : jugadorN2->getTorres()) {
        QList<Enemigo*> enemigosEnRango;

        for (Enemigo* enemigo : enemigos) {
            if (enemigo && enemigo->estaViva()) {
                QPointF distancia = enemigo->getPosicion() - torre->getPosicion();
                float magnitud = qSqrt(distancia.x() * distancia.x() + distancia.y() * distancia.y());

                if (magnitud <= torre->getRango()) {
                    enemigosEnRango.append(enemigo);
                }
            }
        }

        // Actualizar la torre
        torre->actualizar(16); // deltaTime aproximado
    }
}

void Nivel2::generarOleada()
{
    // Verificar si el juego está pausado
    if (!timerJuego->isActive()) return;

    int cantidadEnemigos = 3 + (numeroOleada * 2);

    qDebug() << "Generando oleada" << numeroOleada << "con" << cantidadEnemigos << "enemigos";

    for (int i = 0; i < cantidadEnemigos; ++i) {
        // Pequeño delay entre generación de enemigos
        QTimer::singleShot(i * 500, this, [this]() {
            generarEnemigo();
        });
    }

    numeroOleada++;

    // Aumentar dificultad progresivamente
    int nuevoIntervalo = qMax(3000, 10000 - (numeroOleada * 500));
    timerOleadas->setInterval(nuevoIntervalo);
}

void Nivel2::generarEnemigo()
{
    // Verificar si el juego está pausado
    if (!timerJuego->isActive()) return;

    // Seleccionar ruta aleatoria
    int indiceRuta = QRandomGenerator::global()->bounded(rutasEnemigas.size());
    QList<QPointF> ruta = rutasEnemigas[indiceRuta];

    // Crear enemigo - tipo aleatorio con progresión de dificultad
    int tipoEnemigo;
    int random = QRandomGenerator::global()->bounded(100);

    if (random < 70) {
        tipoEnemigo = 1; // Enemigo débil
    } else {
        tipoEnemigo = 2; // Enemigo fuerte
    }

    // Aumentar dificultad según oleada
    if (numeroOleada > 3) {
        tipoEnemigo = 2; // Más enemigos fuertes después de la oleada 3
    }

    Enemigo* nuevoEnemigo = new Enemigo(tipoEnemigo);
    nuevoEnemigo->setPosicion(ruta.first());

    enemigos.append(nuevoEnemigo);

    qDebug() << "Enemigo generado en ruta" << indiceRuta << "tipo:" << tipoEnemigo;
}

void Nivel2::generarRecursos()
{
    // Verificar si el juego está pausado
    if (!timerJuego->isActive()) return;

    int recursosBase = 10 + (numeroOleada * 2);
    JugadorNivel2* jugadorN2 = static_cast<JugadorNivel2*>(jugador);
    jugadorN2->agregarRecursos(recursosBase);
    recursosGenerados += recursosBase;

    qDebug() << "Recursos generados:" << recursosBase << "Total del nivel:" << recursosGenerados;
}

void Nivel2::procesarColisiones()
{
    JugadorNivel2* jugadorN2 = static_cast<JugadorNivel2*>(jugador);

    // Colisión entre enemigos y castillo
    QRectF areaCastillo(412, 284, 200, 200); // Área del castillo

    for (Enemigo* enemigo : enemigos) {
        if (enemigo->estaViva() && areaCastillo.intersects(enemigo->getAreaColision())) {
            // El enemigo llegó al castillo - causar daño
            float danio = 10.0f; // Daño base
            if (enemigo->getTipo() == 2) {
                danio = 20.0f; // Enemigo fuerte hace más daño
            }

            jugador->recibirDanio(danio);

            // Eliminar enemigo
            enemigo->recibirDanio(enemigo->getVida()); // Matar instantáneamente

            qDebug() << "¡Castillo dañado! Vida restante:" << jugador->getVida();
        }
    }
}

void Nivel2::limpiarEnemigosMuertos()
{
    for (int i = enemigos.size() - 1; i >= 0; --i) {
        if (!enemigos[i]->estaViva()) {
            // Otorgar recursos por enemigo eliminado
            JugadorNivel2* jugadorN2 = static_cast<JugadorNivel2*>(jugador);
            int recursosGanados = enemigos[i]->getTipo() == 1 ? 5 : 10; // Más recursos por enemigo fuerte
            jugadorN2->agregarRecursos(recursosGanados);

            delete enemigos[i];
            enemigos.removeAt(i);
        }
    }
}

void Nivel2::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Dibujar fondo
    painter.fillRect(rect(), QColor(34, 139, 34)); // Verde bosque

    // Dibujar rutas
    dibujarRutas(painter);

    // Dibujar castillo
    dibujarCastillo(painter);

    // Dibujar torres
    dibujarTorres(painter);

    // Dibujar enemigos
    for (Enemigo* enemigo : enemigos) {
        if (enemigo->estaViva()) {
            // Dibujar enemigo directamente aquí
            QPointF pos = enemigo->getPosicion();
            int tipo = enemigo->getTipo();

            if (tipo == 1) {
                // Enemigo débil - rojo
                painter.setBrush(QColor(220, 20, 60));
                painter.setPen(QPen(Qt::black, 1));
                painter.drawEllipse(pos, 8, 8);
            } else {
                // Enemigo fuerte - morado
                painter.setBrush(QColor(138, 43, 226));
                painter.setPen(QPen(Qt::black, 1));
                painter.drawEllipse(pos, 12, 12);
            }

            // Barra de vida (usando vida actual como porcentaje)
            float vidaMaxima = (tipo == 1) ? 25.0f : 70.0f; // Valores del constructor
            float vidaPorcentaje = enemigo->getVida() / vidaMaxima;
            QRectF barraFondo(pos.x() - 10, pos.y() - 15, 20, 3);
            QRectF barraVida(pos.x() - 10, pos.y() - 15, 20 * vidaPorcentaje, 3);

            painter.setBrush(Qt::red);
            painter.drawRect(barraFondo);
            painter.setBrush(Qt::green);
            painter.drawRect(barraVida);
        }
    }

    // Dibujar interfaz de construcción si está activa
    if (modoConstruccion) {
        dibujarInterfazConstruccion(painter);
    }

    // Dibujar HUD
    dibujarHUD(painter);
}

void Nivel2::dibujarRutas(QPainter &painter)
{
    painter.setPen(QPen(QColor(139, 69, 19), 80)); // Marrón ancho para caminos
    painter.setBrush(Qt::NoBrush);

    for (const QList<QPointF>& ruta : rutasEnemigas) {
        if (ruta.size() < 2) continue;

        QPainterPath path;
        path.moveTo(ruta[0]);

        for (int i = 1; i < ruta.size(); ++i) {
            path.lineTo(ruta[i]);
        }

        painter.drawPath(path);
    }
}

void Nivel2::dibujarCastillo(QPainter &painter)
{
    // Base del castillo
    painter.setBrush(QColor(101, 67, 33)); // Marrón
    painter.setPen(QPen(Qt::black, 2));
    painter.drawRect(412, 284, 200, 200);

    // Torres del castillo
    painter.setBrush(QColor(128, 128, 128)); // Gris
    painter.drawRect(402, 264, 20, 40);
    painter.drawRect(602, 264, 20, 40);
    painter.drawRect(412, 244, 60, 40);
    painter.drawRect(552, 244, 60, 40);

    // Bandera
    painter.setBrush(Qt::red);
    painter.drawRect(507, 220, 10, 30);

    // Puerta
    painter.setBrush(QColor(101, 67, 33));
    painter.drawRect(482, 384, 60, 100);
    painter.setBrush(QColor(139, 69, 19));
    painter.drawRect(492, 394, 40, 80);
}

void Nivel2::dibujarTorres(QPainter &painter)
{
    JugadorNivel2* jugadorN2 = static_cast<JugadorNivel2*>(jugador);

    for (Torre* torre : jugadorN2->getTorres()) {
        QPointF pos = torre->getPosicion();
        float rango = torre->getRango();

        // Dibujar rango (semi-transparente)
        painter.setBrush(QColor(255, 255, 255, 30));
        painter.setPen(QPen(QColor(255, 255, 255, 80), 1));
        painter.drawEllipse(pos, rango, rango);

        // Dibujar base de la torre
        painter.setBrush(QColor(128, 128, 128));
        painter.setPen(QPen(Qt::black, 2));
        painter.drawEllipse(pos, 25, 25);

        // Dibujar torre según tipo
        switch(torre->getTipo()) {
        case Arma::ARCO:
            painter.setBrush(QColor(160, 82, 45)); // Marrón
            break;
        case Arma::BALLESTA:
            painter.setBrush(QColor(139, 69, 19)); // Marrón oscuro
            break;
        case Arma::CATAPULTA:
            painter.setBrush(QColor(101, 67, 33)); // Marrón muy oscuro
            break;
        case Arma::MAGICA:
            painter.setBrush(QColor(75, 0, 130)); // Índigo
            break;
        default:
            painter.setBrush(Qt::gray);
            break;
        }

        painter.drawRect(pos.x() - 15, pos.y() - 15, 30, 30);

        // Indicador de nivel
        painter.setPen(Qt::white);
        painter.drawText(pos.x() - 5, pos.y() + 5, QString::number(torre->getNivel()));
    }
}

void Nivel2::dibujarInterfazConstruccion(QPainter &painter)
{
    // Panel de información de construcción
    painter.setBrush(QColor(0, 0, 0, 180));
    painter.setPen(QPen(Qt::white, 2));
    painter.drawRect(10, 10, 200, 150);

    painter.setPen(Qt::white);
    painter.drawText(20, 30, "MODO CONSTRUCCIÓN");
    painter.drawText(20, 50, "Clic: Construir torre");
    painter.drawText(20, 70, "1-4: Tipo de torre");
    painter.drawText(20, 90, "C: Cambiar modo");
    painter.drawText(20, 110, "Recursos: " + QString::number(static_cast<JugadorNivel2*>(jugador)->getRecursos()));

    // Información de la torre seleccionada
    QString tipoTorre;
    switch(tipoTorreSeleccionado) {
    case Arma::ARCO: tipoTorre = "Arco (40)"; break;
    case Arma::BALLESTA: tipoTorre = "Ballesta (60)"; break;
    case Arma::CATAPULTA: tipoTorre = "Catapulta (100)"; break;
    case Arma::MAGICA: tipoTorre = "Mágica (80)"; break;
    default: tipoTorre = "Desconocida"; break;
    }

    painter.drawText(20, 130, "Torre: " + tipoTorre);
}

void Nivel2::dibujarHUD(QPainter &painter)
{
    // Panel superior de información del juego
    painter.setBrush(QColor(0, 0, 0, 180));
    painter.setPen(QPen(Qt::white, 2));
    painter.drawRect(rect().width() - 210, 10, 200, 100);

    painter.setPen(Qt::white);
    painter.drawText(rect().width() - 200, 30, "Oleada: " + QString::number(numeroOleada));
    painter.drawText(rect().width() - 200, 50, "Tiempo: " + QString::number((int)tiempoTranscurrido) + "/" + QString::number(tiempoObjetivo));
    painter.drawText(rect().width() - 200, 70, "Vida Castillo: " + QString::number((int)jugador->getVida()));
    painter.drawText(rect().width() - 200, 90, "Torres: " + QString::number(static_cast<JugadorNivel2*>(jugador)->getTorres().size()));
}

void Nivel2::mousePressEvent(QMouseEvent *event)
{
    // Verificar si el juego está pausado
    if (!timerJuego->isActive()) return;

    if (modoConstruccion && event->button() == Qt::LeftButton) {
        QPointF posicionMundo = event->pos();
        procesarClickConstruccion(posicionMundo);
    }

    NivelBase::mousePressEvent(event);
}

void Nivel2::keyPressEvent(QKeyEvent *event)
{
    // Verificar si el juego está pausado
    if (!timerJuego->isActive()) {
        NivelBase::keyPressEvent(event);
        return;
    }

    switch(event->key()) {
    case Qt::Key_1:
        tipoTorreSeleccionado = Arma::ARCO;
        qDebug() << "Torre seleccionada: Arco";
        break;
    case Qt::Key_2:
        tipoTorreSeleccionado = Arma::BALLESTA;
        qDebug() << "Torre seleccionada: Ballesta";
        break;
    case Qt::Key_3:
        tipoTorreSeleccionado = Arma::CATAPULTA;
        qDebug() << "Torre seleccionada: Catapulta";
        break;
    case Qt::Key_4:
        tipoTorreSeleccionado = Arma::MAGICA;
        qDebug() << "Torre seleccionada: Mágica";
        break;
    case Qt::Key_C:
        modoConstruccion = !modoConstruccion;
        qDebug() << "Modo construcción:" << (modoConstruccion ? "ACTIVADO" : "DESACTIVADO");
        break;
    case Qt::Key_Space:
        // Construir torre en posición predefinida (para testing)
        if (modoConstruccion) {
            QPointF posicion(200, 200);
            if (esPosicionValidaParaTorre(posicion)) {
                static_cast<JugadorNivel2*>(jugador)->construirTorre(tipoTorreSeleccionado, posicion);
            }
        }
        break;
    default:
        NivelBase::keyPressEvent(event);
        break;
    }

    update();
}

void Nivel2::procesarClickConstruccion(const QPointF& posicionMundo)
{
    JugadorNivel2* jugadorN2 = static_cast<JugadorNivel2*>(jugador);

    // Primero verificar si se hizo clic en una torre existente
    Torre* torreExistente = jugadorN2->getTorreEnPosicion(posicionMundo);
    if (torreExistente) {
        // Si hay recursos, mejorar la torre
        jugadorN2->mejorarTorre(torreExistente);
        return;
    }

    // Si no, intentar construir nueva torre
    if (esPosicionValidaParaTorre(posicionMundo)) {
        bool exito = jugadorN2->construirTorre(tipoTorreSeleccionado, posicionMundo);
        if (exito) {
            qDebug() << "Torre construida exitosamente en" << posicionMundo;
        } else {
            qDebug() << "No hay recursos suficientes para construir torre";
        }
    } else {
        qDebug() << "Posición inválida para construir torre";
    }
}

bool Nivel2::esPosicionValidaParaTorre(const QPointF& posicion) const
{
    // Verificar que no esté demasiado cerca del castillo
    QPointF centroCastillo(512, 384);
    QPointF distanciaAlCastillo = posicion - centroCastillo;
    float distancia = qSqrt(distanciaAlCastillo.x() * distanciaAlCastillo.x() +
                            distanciaAlCastillo.y() * distanciaAlCastillo.y());

    if (distancia < 200) {
        return false; // Demasiado cerca del castillo
    }

    // Verificar que no esté en una ruta
    for (const QList<QPointF>& ruta : rutasEnemigas) {
        for (const QPointF& punto : ruta) {
            QPointF distanciaARuta = posicion - punto;
            float dist = qSqrt(distanciaARuta.x() * distanciaARuta.x() +
                               distanciaARuta.y() * distanciaARuta.y());
            if (dist < 60) { // Radio de la ruta + margen
                return false; // Está en una ruta
            }
        }
    }

    // Verificar que no esté demasiado cerca de otra torre
    const JugadorNivel2* jugadorN2 = static_cast<const JugadorNivel2*>(jugador);
    for (const Torre* torre : jugadorN2->getTorres()) {
        QPointF distanciaATorre = posicion - torre->getPosicion();
        float dist = qSqrt(distanciaATorre.x() * distanciaATorre.x() +
                           distanciaATorre.y() * distanciaATorre.y());
        if (dist < 80) { // Distancia mínima entre torres
            return false;
        }
    }

    return true;
}
