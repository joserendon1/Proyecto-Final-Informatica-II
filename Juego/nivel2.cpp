#include "nivel2.h"
#include "audiomanager.h"
#include "spritemanager.h"
#include "uimanager.h"
#include <QPainter>
#include <QKeyEvent>
#include <QTimer>
#include <QDebug>
#include <QRandomGenerator>

Nivel2::Nivel2(QWidget *parent) : NivelBase(parent)
{
    jugador = new JugadorNivel2();
    timerGeneracionObstaculos = new QTimer(this);

    timerJuego->setInterval(16);
    connect(timerGeneracionObstaculos, &QTimer::timeout, this, &Nivel2::generarObstaculo);

    tiempoTranscurrido = 0;
    tiempoObjetivo = 90;

    qDebug() << "Nivel 2 inicializado - Esquiva obstáculos";
}

Nivel2::~Nivel2()
{
    obstaculos.clear();

    if (timerGeneracionObstaculos) {
        timerGeneracionObstaculos->stop();
        delete timerGeneracionObstaculos;
    }

    qDebug() << "Nivel 2 destruido";
}

void Nivel2::iniciarNivel()
{
    NivelBase::iniciarNivel();

    AudioManager::getInstance().stopBackgroundMusic();
    timerGeneracionObstaculos->start(frecuenciaGeneracion);

    // Limpiar obstáculos existentes y resetear estado
    obstaculos.clear();
    obstaculosEsquivados = 0;
    tiempoTranscurrido = 0;

    qDebug() << "Nivel 2 iniciado - Esquiva obstáculos por 90 segundos";
    update();
}

void Nivel2::pausarNivel()
{
    NivelBase::pausarNivel();
    timerGeneracionObstaculos->stop();
    update();
}

void Nivel2::reanudarNivel()
{
    NivelBase::reanudarNivel();
    timerGeneracionObstaculos->start();
    update();
}

void Nivel2::actualizarJuego(float deltaTime)
{
    if (!timerJuego->isActive()) return;

    // Actualizar tiempo
    tiempoTranscurrido += deltaTime / 1000.0f;

    // Verificar victoria
    if (tiempoTranscurrido >= tiempoObjetivo) {
        emit levelCompleted();
        pausarNivel();
        return;
    }

    // Actualizar jugador
    jugador->actualizar(deltaTime);

    // Actualizar obstáculos (mover hacia abajo)
    for (Obstaculo& obstaculo : obstaculos) {
        if (obstaculo.activo) {
            obstaculo.posicion.setY(obstaculo.posicion.y() + obstaculo.velocidad);
        }
    }

    // Procesar colisiones
    procesarColisiones();

    // Limpiar obstáculos que salieron de pantalla
    limpiarObstaculos();

    // Aumentar dificultad progresivamente
    if (tiempoTranscurrido > 30 && frecuenciaGeneracion > 500) {
        frecuenciaGeneracion = 1000 - (tiempoTranscurrido * 10);
        if (frecuenciaGeneracion < 500) frecuenciaGeneracion = 500;
        timerGeneracionObstaculos->setInterval(frecuenciaGeneracion);
    }

    // Verificar derrota
    if (jugador->getVida() <= 0) {
        emit gameOver();
        pausarNivel();
        return;
    }

    update();
}

void Nivel2::generarObstaculo()
{
    if (!timerJuego->isActive()) return;

    // Determinar tipo de obstáculo basado en probabilidad
    int tipo;
    float velocidad;
    int random = QRandomGenerator::global()->bounded(100);

    if (random < 60) {
        tipo = 1; // Normal
        velocidad = 3.0f;
    } else if (random < 85) {
        tipo = 2; // Grande
        velocidad = 2.5f;
    } else {
        tipo = 3; // Rápido
        velocidad = 5.0f;
    }

    // Posición aleatoria en X
    float posX = QRandomGenerator::global()->bounded(100, 924);
    QPointF posicion(posX, -50);

    // Crear obstáculo directamente en la lista
    obstaculos.append(Obstaculo(posicion, tipo, velocidad));
}

void Nivel2::procesarColisiones()
{
    for (Obstaculo& obstaculo : obstaculos) {
        if (obstaculo.activo && obstaculo.getAreaColision().intersects(jugador->getAreaColision())) {
            // Colisión detectada - aplicar daño
            float danio = 25.0f;
            jugador->recibirDanio(danio);

            // Desactivar obstáculo
            obstaculo.activo = false;

            qDebug() << "¡Colisión! Vida restante:" << jugador->getVida();

            // Reproducir sonido de colisión si está disponible
            AudioManager::getInstance().playSound("hit");
        }
    }
}

void Nivel2::limpiarObstaculos()
{
    for (int i = obstaculos.size() - 1; i >= 0; --i) {
        if (!obstaculos[i].activo || obstaculos[i].estaFueraDePantalla()) {
            if (obstaculos[i].estaFueraDePantalla() && obstaculos[i].activo) {
                obstaculosEsquivados++;
            }
            obstaculos.removeAt(i);
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
    painter.setBrush(QColor(34, 139, 34)); // Verde pasto
    painter.setPen(Qt::NoPen);
    painter.drawRect(0, 700, width(), 68);

    // Dibujar jugador
    dibujarJugador(painter);

    // Dibujar obstáculos
    dibujarObstaculos(painter);

    // Dibujar HUD
    dibujarHUD(painter);
}

void Nivel2::dibujarJugador(QPainter &painter)
{
    QPointF pos = jugador->getPosicion();

    // Cuerpo del jugador (cuadrado azul)
    painter.setBrush(QColor(0, 100, 255));
    painter.setPen(QPen(Qt::black, 2));
    painter.drawRect(QRectF(pos.x() - 20, pos.y() - 20, 40, 40));

    // Ojos del jugador
    painter.setBrush(Qt::white);
    painter.drawEllipse(QPointF(pos.x() - 8, pos.y() - 5), 5, 5);
    painter.drawEllipse(QPointF(pos.x() + 8, pos.y() - 5), 5, 5);

    // Pupilas
    painter.setBrush(Qt::black);
    painter.drawEllipse(QPointF(pos.x() - 8, pos.y() - 5), 2, 2);
    painter.drawEllipse(QPointF(pos.x() + 8, pos.y() - 5), 2, 2);

    // Sonrisa
    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawArc(QRectF(pos.x() - 10, pos.y(), 20, 10), 0, -180 * 16);
}

void Nivel2::dibujarObstaculos(QPainter &painter)
{
    for (const Obstaculo& obstaculo : obstaculos) {
        if (obstaculo.activo) {
            QPointF pos = obstaculo.posicion;
            int tipo = obstaculo.tipo;

            // Color según tipo
            switch(tipo) {
            case 1: // Normal - Rojo
                painter.setBrush(QColor(200, 0, 0));
                break;
            case 2: // Grande - Rojo oscuro
                painter.setBrush(QColor(139, 0, 0));
                break;
            case 3: // Rápido - Rojo anaranjado
                painter.setBrush(QColor(255, 69, 0));
                break;
            }

            painter.setPen(QPen(Qt::black, 2));

            // Dibujar según tipo
            switch(tipo) {
            case 1: // Normal - Círculo
                painter.drawEllipse(pos, 15, 15);
                // Detalle interior
                painter.setBrush(QColor(255, 100, 100));
                painter.drawEllipse(pos, 8, 8);
                break;
            case 2: // Grande - Cuadrado
                painter.drawRect(QRectF(pos.x() - 25, pos.y() - 25, 50, 50));
                // Detalle interior
                painter.setBrush(QColor(200, 50, 50));
                painter.drawRect(QRectF(pos.x() - 15, pos.y() - 15, 30, 30));
                break;
            case 3: // Rápido - Triángulo
                QPolygonF triangulo;
                triangulo << QPointF(pos.x(), pos.y() - 10)
                          << QPointF(pos.x() - 10, pos.y() + 10)
                          << QPointF(pos.x() + 10, pos.y() + 10);
                painter.drawPolygon(triangulo);
                break;
            }
        }
    }
}

void Nivel2::dibujarHUD(QPainter &painter)
{
    // Panel de información superior izquierdo
    painter.setBrush(QColor(0, 0, 0, 180));
    painter.setPen(QPen(Qt::white, 2));
    painter.drawRect(10, 10, 250, 80);

    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 12, QFont::Bold));

    painter.drawText(20, 30, "TIEMPO: " + QString::number((int)tiempoTranscurrido) + "/" + QString::number(tiempoObjetivo));
    painter.drawText(20, 50, "VIDA: " + QString::number((int)jugador->getVida()));
    painter.drawText(20, 70, "ESQUIVADOS: " + QString::number(obstaculosEsquivados));

    // Instrucciones superior derecho
    painter.drawText(rect().width() - 200, 30, "CONTROLES:");
    painter.drawText(rect().width() - 200, 50, "A - Mover Izquierda");
    painter.drawText(rect().width() - 200, 70, "D - Mover Derecha");

    // Barra de progreso del tiempo
    float progreso = tiempoTranscurrido / (float)tiempoObjetivo;
    QRectF barraFondo(rect().center().x() - 100, 20, 200, 15);
    QRectF barraProgreso(rect().center().x() - 100, 20, 200 * progreso, 15);

    painter.setBrush(QColor(100, 100, 100));
    painter.drawRect(barraFondo);
    painter.setBrush(QColor(0, 255, 0));
    painter.drawRect(barraProgreso);
    painter.setPen(Qt::white);
    painter.drawText(barraFondo, Qt::AlignCenter, "PROGRESO");
}

void Nivel2::keyPressEvent(QKeyEvent *event)
{
    if (!timerJuego->isActive()) {
        NivelBase::keyPressEvent(event);
        return;
    }

    std::vector<bool> teclas(4, false);

    switch(event->key()) {
    case Qt::Key_A:
        teclas[0] = true; // Izquierda
        break;
    case Qt::Key_D:
        teclas[1] = true; // Derecha
        break;
    case Qt::Key_P:
        pausarNivel();
        break;
    case Qt::Key_R:
        reanudarNivel();
        break;
    default:
        NivelBase::keyPressEvent(event);
        return;
    }

    jugador->procesarInput(teclas);
    update();
}
