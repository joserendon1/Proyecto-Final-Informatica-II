#include "nivelbase.h"
#include <QKeyEvent>
#include <QDateTime>
#include <QDebug>

NivelBase::NivelBase(QWidget *parent) : QWidget(parent)
{
    // Inicializar pointers
    jugador = nullptr;
    mapa = nullptr;
    timerJuego = nullptr;

    // Configuración base de la vista
    tamanoVista = QSize(1024, 768);

    // Inicializar teclas
    teclas.resize(4, false);

    setupNivelBase();
}

NivelBase::~NivelBase()
{
    if (timerJuego) {
        timerJuego->stop();
        delete timerJuego;
    }
    if (jugador) {
        delete jugador;
    }
    if (mapa) {
        delete mapa;
    }
}

void NivelBase::setupNivelBase()
{
    setFixedSize(tamanoVista);
    setFocusPolicy(Qt::StrongFocus);

    // Crear mapa base
    mapa = new Mapa(this);

    // Timer común
    setupTimer();

    tiempoUltimoFrame = QDateTime::currentMSecsSinceEpoch();
}

void NivelBase::setupTimer()
{
    timerJuego = new QTimer(this);
    connect(timerJuego, &QTimer::timeout, this, [this]() {
        qint64 tiempoActual = QDateTime::currentMSecsSinceEpoch();
        float deltaTime = tiempoActual - tiempoUltimoFrame;
        tiempoUltimoFrame = tiempoActual;

        this->actualizarJuego(deltaTime);
        this->update();
    });
}

void NivelBase::actualizarCamara()
{
    if (!jugador) return;

    QPointF objetivo = jugador->getPosicion() - QPointF(tamanoVista.width()/2, tamanoVista.height()/2);
    posicionCamara += (objetivo - posicionCamara) * 0.05f;

    QRectF limitesMapa = mapa->getLimitesMapa();
    posicionCamara.setX(qMax(limitesMapa.left(), qMin(posicionCamara.x(), limitesMapa.right() - tamanoVista.width())));
    posicionCamara.setY(qMax(limitesMapa.top(), qMin(posicionCamara.y(), limitesMapa.bottom() - tamanoVista.height())));
}

QRectF NivelBase::getVistaCamara() const
{
    return QRectF(posicionCamara, tamanoVista);
}

bool NivelBase::estaEnVista(const QPointF& posicion) const
{
    QRectF vista = getVistaCamara();
    return vista.contains(posicion);
}

bool NivelBase::estaEnVista(const QRectF& area) const
{
    QRectF vista = getVistaCamara();
    return vista.intersects(area);
}

void NivelBase::keyPressEvent(QKeyEvent *event)
{
    // Input base común (WASD)
    switch(event->key()) {
    case Qt::Key_W: teclas[0] = true; break;
    case Qt::Key_A: teclas[1] = true; break;
    case Qt::Key_S: teclas[2] = true; break;
    case Qt::Key_D: teclas[3] = true; break;
    case Qt::Key_P: pausarNivel(); break;
    case Qt::Key_R: reanudarNivel(); break;
    }
}

void NivelBase::keyReleaseEvent(QKeyEvent *event)
{
    switch(event->key()) {
    case Qt::Key_W: teclas[0] = false; break;
    case Qt::Key_A: teclas[1] = false; break;
    case Qt::Key_S: teclas[2] = false; break;
    case Qt::Key_D: teclas[3] = false; break;
    }
}

void NivelBase::resetearTeclas()
{
    std::fill(teclas.begin(), teclas.end(), false);
}

void NivelBase::iniciarNivel()
{
    if (timerJuego) {
        timerJuego->start(16);
    }
}

void NivelBase::pausarNivel()
{
    if (timerJuego) {
        timerJuego->stop();
    }
    resetearTeclas();
    emit gamePaused();
}

void NivelBase::reanudarNivel()
{
    if (timerJuego) {
        timerJuego->start(16);
    }
    emit gameResumed();
}
