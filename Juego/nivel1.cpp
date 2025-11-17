#include "nivel1.h"
#include <QPainter>
#include <QDebug>
#include <QtMath>
#include <QInputDialog>

Nivel1::Nivel1(QWidget *parent) : QWidget(parent)
{
    // Configurar ventana
    setFixedSize(800, 600);
    setFocusPolicy(Qt::StrongFocus);

    // Inicializar jugador
    jugador = new JugadorNivel1();

    // Inicializar timers
    timerJuego = new QTimer(this);
    timerOleadas = new QTimer(this);
    tiempoUltimoFrame = QDateTime::currentMSecsSinceEpoch();

    connect(timerJuego, &QTimer::timeout, this, &Nivel1::actualizarJuego);
    connect(timerOleadas, &QTimer::timeout, this, &Nivel1::generarOleada);

    // Inicializar variables
    for(int i = 0; i < 4; i++) {
        teclas[i] = false;
    }

    tiempoTranscurrido = 0;
    tiempoObjetivo = 120;
    numeroOleada = 1;
    enemigosPorOleada = 2;
    frecuenciaGeneracion = 5000;

    mostrandoMejoras = false;

    // Inicializar sistema de mejoras
    inicializarMejoras();

    iniciarNivel();
}

Nivel1::~Nivel1()
{
    qDeleteAll(enemigos);
    enemigos.clear();
    delete jugador;
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
}

void Nivel1::reanudarNivel()
{
    timerJuego->start(16);
    timerOleadas->start(frecuenciaGeneracion);
}

void Nivel1::inicializarMejoras()
{
    todasLasMejoras.clear();

    // Mejoras de estadísticas
    todasLasMejoras.append(Mejora(Mejora::VIDA, "Vida Reforzada", "+25 Vida", 25.0f));
    todasLasMejoras.append(Mejora(Mejora::VIDA, "Corazón Guerrero", "+40 Vida", 40.0f));
    todasLasMejoras.append(Mejora(Mejora::DANIO, "Filo Mejorado", "+5 Daño", 5.0f));
    todasLasMejoras.append(Mejora(Mejora::DANIO, "Acero Templado", "+10 Daño", 10.0f));
    todasLasMejoras.append(Mejora(Mejora::VELOCIDAD, "Botas Ligeras", "+1 Velocidad", 1.0f));
    todasLasMejoras.append(Mejora(Mejora::VELOCIDAD, "Viento Rápido", "+2 Velocidad", 2.0f));

    // NUEVAS: Mejoras de armas (tipo ARMA con valor de tipoArma)
    todasLasMejoras.append(Mejora(Mejora::ARMA, "Ballesta", "Dispara flechas hacia adelante", 0, Arma::BALLESTA));
    todasLasMejoras.append(Mejora(Mejora::ARMA, "Arco Multidireccional", "Dispara en 4 direcciones", 0, Arma::ARCO));
    todasLasMejoras.append(Mejora(Mejora::ARMA, "Lanza Larga", "Ataque en línea recta", 0, Arma::LANZA));
    todasLasMejoras.append(Mejora(Mejora::ARMA, "Aceite Hirviendo", "Área de daño alrededor", 0, Arma::ACEITE));
    todasLasMejoras.append(Mejora(Mejora::ARMA, "Escudo Protector", "Empuja enemigos cercanos", 0, Arma::ESCUDO));
}

QList<Mejora> Nivel1::generarOpcionesMejoras(int cantidad)
{
    QList<Mejora> opciones;
    QList<Mejora> disponibles = todasLasMejoras;

    // Filtrar armas que el jugador ya tiene
    for(int i = disponibles.size() - 1; i >= 0; i--) {
        if(disponibles[i].getTipo() == Mejora::ARMA) {
            if(jugador->tieneArma(static_cast<Arma::Tipo>(disponibles[i].getTipoArma()))) {
                disponibles.removeAt(i);
            }
        }
    }

    if(disponibles.isEmpty()) {
        // Si no hay mejoras disponibles, regenerar todas
        inicializarMejoras();
        disponibles = todasLasMejoras;
    }

    // Mezclar las mejoras disponibles
    for(int i = disponibles.size() - 1; i > 0; --i) {
        int j = QRandomGenerator::global()->bounded(i + 1);
        disponibles.swapItemsAt(i, j);
    }

    // Tomar las primeras 'cantidad' mejoras
    for(int i = 0; i < qMin(cantidad, disponibles.size()); ++i) {
        opciones.append(disponibles[i]);
    }

    return opciones;
}

void Nivel1::aplicarMejora(const Mejora& mejora)
{
    mejora.aplicar(jugador);

    // Mostrar información de la mejora aplicada
    QString mensaje = QString("Mejora aplicada: %1 - %2").arg(mejora.getNombre()).arg(mejora.getDescripcion());

    // Mostrar en consola
    qDebug() << mensaje;

    // También podrías mostrarlo en pantalla temporalmente
    // (lo implementaremos después en el HUD)
}

void Nivel1::mostrarOpcionesMejoras()
{
    mostrandoMejoras = true;
    resetearTeclas();
    pausarNivel();

    QList<Mejora> opciones = generarOpcionesMejoras(3);

    QString mensaje = "¡Subiste de nivel!\nElige una mejora:\n\n";
    for(int i = 0; i < opciones.size(); ++i) {
        const Mejora& mejora = opciones[i];
        QString tipo = "";

        switch(mejora.getTipo()) {
        case Mejora::VIDA: tipo = "❤️ Vida"; break;
        case Mejora::DANIO: tipo = "⚔️ Daño"; break;
        case Mejora::VELOCIDAD: tipo = "👟 Velocidad"; break;
        case Mejora::ARMA: tipo = "🎯 Arma"; break;
        }

        mensaje += QString("%1. [%2] %3 - %4\n")
                       .arg(i + 1)
                       .arg(tipo)
                       .arg(mejora.getNombre())
                       .arg(mejora.getDescripcion());
    }
    mensaje += "\nPresiona 1, 2 o 3 para elegir";

    QMessageBox::information(this, "¡Subiste de Nivel!", mensaje);
}

void Nivel1::actualizarJuego()
{
    // Calcular deltaTime
    qint64 tiempoActual = QDateTime::currentMSecsSinceEpoch();
    float deltaTime = tiempoActual - tiempoUltimoFrame;
    tiempoUltimoFrame = tiempoActual;

    // Si estamos mostrando mejoras, no actualizar el juego
    if(mostrandoMejoras) {
        return;
    }

    // Actualizar tiempo del juego (ahora con deltaTime)
    static float acumuladorTiempo = 0;
    acumuladorTiempo += deltaTime;
    if(acumuladorTiempo >= 1000) {
        tiempoTranscurrido++;
        acumuladorTiempo = 0;
    }

    // Actualizar jugador con deltaTime
    jugador->procesarInput(teclas);
    jugador->actualizar(deltaTime);

    // Actualizar enemigos con deltaTime
    for(Enemigo *enemigo : enemigos) {
        enemigo->seguirJugador(jugador->getPosicion());
        enemigo->actualizar(deltaTime);
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
    }

    if(!jugador->estaViva()) {
        timerJuego->stop();
        timerOleadas->stop();
        qDebug() << "Game Over - Has sido derrotado";
    }

    update();
}

void Nivel1::generarOleada()
{
    for(int i = 0; i < enemigosPorOleada; i++) {
        generarEnemigo();
    }

    // Aumentar dificultad más gradualmente
    numeroOleada++;
    enemigosPorOleada += 1;
    frecuenciaGeneracion = qMax(2000, frecuenciaGeneracion - 100); // REDUCIDO de -200 a -100
    timerOleadas->setInterval(frecuenciaGeneracion);

    qDebug() << "Oleada" << numeroOleada << " - Enemigos:" << enemigosPorOleada;
}

void Nivel1::generarEnemigo()
{
    Enemigo *enemigo = new Enemigo();

    // Generar en posición aleatoria en los bordes
    int lado = QRandomGenerator::global()->bounded(4);
    QPointF posicion;

    switch(lado) {
    case 0: // Arriba
        posicion = QPointF(QRandomGenerator::global()->bounded(800), -20);
        break;
    case 1: // Derecha
        posicion = QPointF(820, QRandomGenerator::global()->bounded(600));
        break;
    case 2: // Abajo
        posicion = QPointF(QRandomGenerator::global()->bounded(800), 620);
        break;
    case 3: // Izquierda
        posicion = QPointF(-20, QRandomGenerator::global()->bounded(600));
        break;
    }

    enemigo->setPosicion(posicion);

    // Hacer enemigos más fuertes en oleadas posteriores
    if(numeroOleada > 3) {
        enemigo->setVida(50.0f);
    }

    enemigos.append(enemigo);
}

void Nivel1::procesarColisiones()
{
    // Procesar colisiones armas-enemigos
    for(Arma* arma : jugador->getArmas()) {
        QList<QRectF> areasAtaque = arma->getAreasAtaque();

        for(const QRectF& area : areasAtaque) {
            for(int i = enemigos.size() - 1; i >= 0; i--) {
                Enemigo* enemigo = enemigos[i];

                if(area.intersects(enemigo->getAreaColision())) {
                    enemigo->recibirDanio(25); // Daño base

                    if(!enemigo->estaViva()) {
                        jugador->ganarExperiencia(enemigo->getExperienciaQueDa());
                        // No eliminar aquí, lo hace limpiarEnemigosMuertos()
                    }
                }
            }
        }
    }

    // Procesar colisiones jugador-enemigos
    for(Enemigo* enemigo : enemigos) {
        if(jugador->getAreaColision().intersects(enemigo->getAreaColision())) {
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
            int indice = event->key() - Qt::Key_1;
            QList<Mejora> opciones = generarOpcionesMejoras(3);

            if(indice >= 0 && indice < opciones.size()) {
                aplicarMejora(opciones[indice]);
                jugador->setMejoraPendiente(false);
                mostrandoMejoras = false;
                resetearTeclas();
                reanudarNivel();
            }
        }
        return;
    }

    // Teclas normales del juego
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

    // Dibujar fondo (simulando murallas)
    painter.fillRect(rect(), QBrush(QColor(60, 60, 80)));

    // Dibujar área de juego central
    painter.fillRect(50, 50, 700, 500, QBrush(QColor(80, 80, 100)));

    // Dibujar jugador (círculo azul temporal)
    painter.setBrush(QBrush(QColor(0, 100, 255)));
    painter.setPen(QPen(Qt::white, 2));
    painter.drawEllipse(jugador->getPosicion(), 15, 15);

    // Dibujar enemigos (círculos rojos temporales)
    painter.setBrush(QBrush(QColor(255, 50, 50)));
    for(Enemigo *enemigo : enemigos) {
        if(enemigo->estaViva()) {
            painter.drawEllipse(enemigo->getPosicion(), 12, 12);
        }
    }

    // Dibujar armas y ataques
    dibujarArmas(painter);

    // Dibujar HUD
    dibujarHUD(painter);
}

void Nivel1::dibujarArmas(QPainter &painter)
{
    for(Arma* arma : jugador->getArmas()) {
        QList<QRectF> areasAtaque = arma->getAreasAtaque();

        // Color diferente según el tipo de arma
        if(arma->getTipo() == Arma::ESPADA) {
            painter.setBrush(QBrush(QColor(255, 255, 0, 80))); // Amarillo para espada
            painter.setPen(QPen(QColor(255, 255, 0), 1));
        } else if(arma->getTipo() == Arma::BALLESTA) {
            painter.setBrush(QBrush(QColor(0, 255, 255, 80))); // Cian para ballesta
            painter.setPen(QPen(QColor(0, 255, 255), 1));
        }

        for(const QRectF& area : areasAtaque) {
            if(arma->getTipo() == Arma::BALLESTA && area.width() == 10) {
                // Es un proyectil - dibujar como círculo
                painter.drawEllipse(area.center(), 5, 5);
            } else {
                // Es un área de ataque - dibujar como rectángulo
                painter.drawRect(area);
            }
        }
    }
}

void Nivel1::dibujarHUD(QPainter &painter)
{
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 12));

    // Información del jugador
    painter.drawText(10, 20, QString("Vida: %1").arg(jugador->getVida()));
    painter.drawText(10, 40, QString("Nivel: %1").arg(jugador->getNivel()));
    painter.drawText(10, 60, QString("EXP: %1/%2").arg(jugador->getExperiencia())
                                 .arg(jugador->getExperienciaParaSiguienteNivel()));

    // Tiempo y oleadas
    painter.drawText(10, 80, QString("Tiempo: %1/%2").arg(tiempoTranscurrido)
                                 .arg(tiempoObjetivo));
    painter.drawText(10, 100, QString("Oleada: %1").arg(numeroOleada));
    painter.drawText(10, 120, QString("Enemigos: %1").arg(enemigos.size()));

    // Mejoras aplicadas
    painter.drawText(10, 140, QString("Daño Extra: +%1").arg(jugador->getDanioExtra()));
    painter.drawText(10, 160, QString("Velocidad Extra: +%1").arg(jugador->getVelocidadExtra()));

    // Controles
    painter.drawText(10, 560, "Controles: WASD - Movimiento");
    painter.drawText(10, 580, "P - Pausa, R - Reanudar");

    // Si hay mejora pendiente, mostrar mensaje
    if(jugador->tieneMejoraPendiente()) {
        painter.setPen(Qt::yellow);
        painter.drawText(300, 30, "¡MEJORA PENDIENTE! Presiona M para elegir");
    }

    // Si estamos mostrando mejoras, indicarlo
    if(mostrandoMejoras) {
        painter.fillRect(rect(), QColor(0, 0, 0, 150)); // Fondo semitransparente
        painter.setPen(Qt::yellow);
        painter.setFont(QFont("Arial", 16, QFont::Bold));
        painter.drawText(rect().center() - QPoint(150, 0), "ELIGE UNA MEJORA (1, 2, 3)");
    }
}
