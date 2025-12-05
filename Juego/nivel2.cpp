#include "nivel2.h"
#include <QPainter>
#include <QKeyEvent>
#include <QDebug>
#include <QRandomGenerator>

Nivel2::Nivel2(QWidget *parent) : QWidget(parent)
{
    setFixedSize(1024, 768);
    jugador = new JugadorNivel2();

    timerJuego = new QTimer(this);
    timerJuego->setInterval(16); // ~60 FPS

    timerGeneracionBarriles = new QTimer(this);
    timerGeneracionBarriles->setInterval(1000); // Generar barril cada segundo

    connect(timerJuego, &QTimer::timeout, this, &Nivel2::actualizarJuego);
    connect(timerGeneracionBarriles, &QTimer::timeout, this, &Nivel2::generarBarril);

    setFocusPolicy(Qt::StrongFocus);
}

Nivel2::~Nivel2()
{
    delete jugador;
    barriles.clear();
}

void Nivel2::iniciarNivel()
{
    barriles.clear();
    tiempoTranscurrido = 0;
    barrilesEsquivados = 0;

    timerJuego->start();
    timerGeneracionBarriles->start();

    update();
}

void Nivel2::pausarNivel()
{
    timerJuego->stop();
    timerGeneracionBarriles->stop();
    enPausa = true;
    emit gamePaused(); // Emitir señal
    update();
}

void Nivel2::reanudarNivel()
{
    timerJuego->start();
    timerGeneracionBarriles->start();
    enPausa = false;
    emit gameResumed(); // Emitir señal
    update();
}

void Nivel2::actualizarJuego()
{
    if (enPausa) return;

    tiempoTranscurrido++;

    // Verificar victoria (90 segundos)
    if (tiempoTranscurrido >= 90 * 60) { // 90 segundos * 60 updates/segundo
        emit levelCompleted();
        pausarNivel();
        return;
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
    if (tiempoTranscurrido > 30 * 60) { // Después de 30 segundos
        timerGeneracionBarriles->setInterval(800);
    }
    if (tiempoTranscurrido > 60 * 60) { // Después de 60 segundos
        timerGeneracionBarriles->setInterval(600);
    }

    // Verificar derrota
    if (jugador->getVida() <= 0) {
        emit gameOver();
        pausarNivel();
        return;
    }

    update();
}

void Nivel2::generarBarril()
{
    if (enPausa) return;

    // Posición aleatoria en X (evitando los bordes)
    float posX = QRandomGenerator::global()->bounded(100, 924);
    QPointF posicion(posX, -50);

    // Velocidad aleatoria entre 3 y 6 (usamos int para evitar ambigüedad)
    int velocidadInt = QRandomGenerator::global()->bounded(30, 61); // 3.0 a 6.0
    float velocidad = velocidadInt / 10.0f;

    barriles.append(Barril(posicion, velocidad));
}

void Nivel2::procesarColisiones()
{
    for (Barril& barril : barriles) {
        if (barril.activo && barril.getAreaColision().intersects(jugador->getAreaColision())) {
            // Colisión detectada
            jugador->recibirDanio(25.0f);
            barril.activo = false;

            qDebug() << "¡Colisión! Vida restante:" << jugador->getVida();
        }
    }
}

void Nivel2::limpiarBarriles()
{
    for (int i = barriles.size() - 1; i >= 0; --i) {
        if (!barriles[i].activo || barriles[i].estaFueraDePantalla()) {
            if (barriles[i].estaFueraDePantalla() && barriles[i].activo) {
                barrilesEsquivados++;
            }
            barriles.removeAt(i);
        }
    }
}

void Nivel2::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Fondo azul cielo
    painter.fillRect(rect(), QColor(135, 206, 235));

    // Dibujar suelo
    painter.setBrush(QColor(34, 139, 34));
    painter.setPen(Qt::NoPen);
    painter.drawRect(0, 700, width(), 68);

    // Dibujar jugador (cuadrado azul)
    QPointF pos = jugador->getPosicion();
    painter.setBrush(QColor(0, 100, 255));
    painter.setPen(QPen(Qt::black, 2));
    painter.drawRect(QRectF(pos.x() - 20, pos.y() - 20, 40, 40));

    // Dibujar barriles (círculos marrones)
    for (const Barril& barril : barriles) {
        if (barril.activo) {
            QPointF pos = barril.posicion;
            painter.setBrush(QColor(139, 69, 19)); // Marrón
            painter.setPen(QPen(Qt::black, 2));
            painter.drawEllipse(pos, 15, 15);

            // Detalle del barril
            painter.setPen(QPen(QColor(101, 67, 33), 2));
            painter.drawLine(QPointF(pos.x() - 10, pos.y()), QPointF(pos.x() + 10, pos.y()));
            painter.drawLine(QPointF(pos.x(), pos.y() - 10), QPointF(pos.x(), pos.y() + 10));
        }
    }

    // HUD (información del juego)
    painter.setBrush(QColor(0, 0, 0, 180));
    painter.setPen(QPen(Qt::white, 2));
    painter.drawRect(10, 10, 250, 80);

    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 12, QFont::Bold));

    painter.drawText(20, 30, "TIEMPO: " + QString::number(tiempoTranscurrido / 60) + "/90");
    painter.drawText(20, 50, "VIDA: " + QString::number((int)jugador->getVida()));
    painter.drawText(20, 70, "ESQUIVADOS: " + QString::number(barrilesEsquivados));

    // Instrucciones
    painter.drawText(rect().width() - 200, 30, "CONTROLES:");
    painter.drawText(rect().width() - 200, 50, "A - Izquierda");
    painter.drawText(rect().width() - 200, 70, "D - Derecha");
    painter.drawText(rect().width() - 200, 90, "P - Pausar");
    painter.drawText(rect().width() - 200, 110, "R - Reanudar");

    if (enPausa) {
        painter.setBrush(QColor(0, 0, 0, 150));
        painter.drawRect(rect());
        painter.setPen(Qt::yellow);
        painter.setFont(QFont("Arial", 36, QFont::Bold));
        painter.drawText(rect(), Qt::AlignCenter, "PAUSA");
    }
}

void Nivel2::keyPressEvent(QKeyEvent *event)
{
    std::vector<bool> teclas(4, false);

    switch(event->key()) {
    case Qt::Key_A:
        teclas[0] = true;
        break;
    case Qt::Key_D:
        teclas[1] = true;
        break;
    case Qt::Key_P:
        pausarNivel();
        break;
    case Qt::Key_R:
        reanudarNivel();
        break;
    default:
        QWidget::keyPressEvent(event);
        return;
    }

    jugador->procesarInput(teclas);
    update();
}
