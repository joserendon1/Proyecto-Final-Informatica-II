#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "audiomanager.h"
#include "mainmenu.h"
#include "nivel1.h"
#include "nivel2.h"
#include "nivel3.h"
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QDebug>
#include <QVBoxLayout>
#include <QScreen>
#include <QApplication>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , nivel1(nullptr)
    , nivel2(nullptr)
    , nivel3(nullptr)
    , menuPrincipal(nullptr)
{
    ui->setupUi(this);

    setWindowTitle("Último Bastión");

    int anchoVentana = 800;
    int altoVentana = 600;

    setFixedSize(anchoVentana, altoVentana);

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - anchoVentana) / 2;
    int y = (screenGeometry.height() - altoVentana) / 4;
    move(x, y);

    setFocusPolicy(Qt::StrongFocus);

    setupMenu();
    setupStatusBar();

    mostrarMenuPrincipal();

    qDebug() << "Ventana configurada - 800x600 fija";
}

MainWindow::~MainWindow()
{
    delete ui;
    limpiarNivelActual();
}

void MainWindow::mostrarMenuPrincipal()
{
    limpiarNivelActual();

    menuPrincipal = new MainMenu(this);
    setCentralWidget(menuPrincipal);

    connect(menuPrincipal, &MainMenu::nivel1Seleccionado, this, &MainWindow::iniciarNivel1);
    connect(menuPrincipal, &MainMenu::nivel2Seleccionado, this, &MainWindow::iniciarNivel2);
    connect(menuPrincipal, &MainMenu::nivel3Seleccionado, this, &MainWindow::iniciarNivel3);
    connect(menuPrincipal, &MainMenu::salir, this, &QMainWindow::close);

    statusBar()->showMessage("Selecciona un nivel para comenzar");
}

void MainWindow::iniciarNivel1()
{
    limpiarNivelActual();
    QTimer::singleShot(100, this, [this]() {
        nivel1 = new Nivel1(this);
        setCentralWidget(nivel1);
        connectGameSignals();

        statusBar()->showMessage("Nivel 1 - Supervivencia: Sobrevive 2 minutos contra oleadas de enemigos");

    });
}

void MainWindow::iniciarNivel2()
{
    limpiarNivelActual();
    QTimer::singleShot(100, this, [this]() {
        nivel2 = new Nivel2(this);
        setCentralWidget(nivel2);
        connectGameSignals();

        statusBar()->showMessage("Nivel 2 - Esquiva: Evita los barriles por 90 segundos");

    });
}

void MainWindow::iniciarNivel3()
{
    limpiarNivelActual();
    QTimer::singleShot(100, this, [this]() {
        nivel3 = new Nivel3(this);
        setCentralWidget(nivel3);
        connectGameSignals();

        statusBar()->showMessage("Nivel 3 - Carrera: Corre y esquiva obstáculos por 60 segundos");

    });
}

void MainWindow::limpiarNivelActual()
{
    AudioManager::getInstance().stopAllSounds();

    if (nivel1) {
        nivel1->pausarNivel();
        delete nivel1;
        nivel1 = nullptr;
    }

    if (nivel2) {
        nivel2->pausarNivel();
        delete nivel2;
        nivel2 = nullptr;
    }

    if (nivel3) {
        nivel3->pausarNivel();
        delete nivel3;
        nivel3 = nullptr;
    }

    if (menuPrincipal) {
        delete menuPrincipal;
        menuPrincipal = nullptr;
    }

    qDebug() << "Nivel actual limpiado y audio detenido";
}

void MainWindow::setupMenu()
{
    QMenu *fileMenu = menuBar()->addMenu("&Archivo");

    QAction *newGameAction = new QAction("&Nuevo Juego", this);
    newGameAction->setShortcut(QKeySequence::New);
    QAction *menuPrincipalAction = new QAction("&Menú Principal", this);
    menuPrincipalAction->setShortcut(Qt::Key_M);
    QAction *exitAction = new QAction("&Salir", this);
    exitAction->setShortcut(QKeySequence::Quit);

    fileMenu->addAction(newGameAction);
    fileMenu->addAction(menuPrincipalAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    connect(newGameAction, &QAction::triggered, this, &MainWindow::onNewGame);
    connect(menuPrincipalAction, &QAction::triggered, this, &MainWindow::mostrarMenuPrincipal);
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);

    QMenu *gameMenu = menuBar()->addMenu("&Juego");

    QAction *pauseAction = new QAction("&Pausar", this);
    pauseAction->setShortcut(Qt::Key_P);
    QAction *resumeAction = new QAction("&Reanudar", this);
    resumeAction->setShortcut(Qt::Key_R);

    gameMenu->addAction(pauseAction);
    gameMenu->addAction(resumeAction);

    connect(pauseAction, &QAction::triggered, this, &MainWindow::onPauseGame);
    connect(resumeAction, &QAction::triggered, this, &MainWindow::onResumeGame);

    QMenu *levelMenu = menuBar()->addMenu("&Nivel");

    QAction *level1Action = new QAction("Nivel &1 - Supervivencia", this);
    QAction *level2Action = new QAction("Nivel &2 - Esquiva", this);  // Cambiado nombre
    QAction *level3Action = new QAction("Nivel &3 - Carrera", this);

    levelMenu->addAction(level1Action);
    levelMenu->addAction(level2Action);
    levelMenu->addAction(level3Action);

    connect(level1Action, &QAction::triggered, this, &MainWindow::iniciarNivel1);
    connect(level2Action, &QAction::triggered, this, &MainWindow::iniciarNivel2);
    connect(level3Action, &QAction::triggered, this, &MainWindow::iniciarNivel3);

    QMenu *helpMenu = menuBar()->addMenu("&Ayuda");

    QAction *controlsAction = new QAction("&Controles", this);
    QAction *aboutAction = new QAction("&Acerca de", this);

    helpMenu->addAction(controlsAction);
    helpMenu->addAction(aboutAction);

    connect(controlsAction, &QAction::triggered, this, &MainWindow::onShowControls);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onShowAbout);
}

void MainWindow::setupStatusBar()
{
    statusBar()->setStyleSheet("QStatusBar { background-color: #2b2b2b; color: white; }");
}

void MainWindow::connectGameSignals()
{
    if (nivel1) {
        connect(nivel1, &Nivel1::gamePaused, this, &MainWindow::onGamePaused);
        connect(nivel1, &Nivel1::gameResumed, this, &MainWindow::onGameResumed);
        connect(nivel1, &Nivel1::playerLevelUp, this, &MainWindow::onPlayerLevelUp);
        connect(nivel1, &Nivel1::gameOver, this, &MainWindow::onGameOver);
        connect(nivel1, &Nivel1::levelCompleted, this, &MainWindow::onLevelCompleted);
    }

    if (nivel2) {
        connect(nivel2, &Nivel2::gamePaused, this, &MainWindow::onGamePaused);
        connect(nivel2, &Nivel2::gameResumed, this, &MainWindow::onGameResumed);
        connect(nivel2, &Nivel2::gameOver, this, &MainWindow::onGameOver);
        connect(nivel2, &Nivel2::levelCompleted, this, &MainWindow::onLevelCompleted);
    }

    if (nivel3) {
        connect(nivel3, &Nivel3::gamePaused, this, &MainWindow::onGamePaused);
        connect(nivel3, &Nivel3::gameResumed, this, &MainWindow::onGameResumed);
        connect(nivel3, &Nivel3::gameOver, this, &MainWindow::onGameOver);
        connect(nivel3, &Nivel3::levelCompleted, this, &MainWindow::onLevelCompleted);
    }
}

void MainWindow::onNewGame()
{

}

void MainWindow::onPauseGame()
{
    if (nivel1) {
        nivel1->pausarNivel();
    } else if (nivel2) {
        nivel2->pausarNivel();
    } else if (nivel3) {
        nivel3->pausarNivel();
    }
}

void MainWindow::onResumeGame()
{
    if (nivel1) {
        nivel1->reanudarNivel();
    } else if (nivel2) {
        nivel2->reanudarNivel();
    } else if (nivel3) {
        nivel3->reanudarNivel();
    }
}

void MainWindow::onShowControls()
{
    QString controles =
        "CONTROLES DEL JUEGO:\n\n"
        "NIVEL 1 - SUPERVIVENCIA:\n"
        "WASD - Movimiento del jugador\n"
        "P - Pausar juego\n"
        "R - Reanudar juego\n"
        "1, 2, 3 - Seleccionar mejoras\n\n"
        "NIVEL 2 - ESQUIVA:\n"
        "A - Mover izquierda\n"
        "D - Mover derecha\n"
        "P - Pausar\n"
        "R - Reanudar\n\n"
        "NIVEL 3 - CARRERA:\n"
        "ESPACIO/W - Saltar obstáculos\n"
        "S - Agacharse\n"
        "P - Pausar, R - Reanudar\n\n"
        "OBJETIVOS:\n"
        "Nivel 1: Sobrevive 2 minutos\n"
        "Nivel 2: Esquiva barriles por 90 segundos\n"
        "Nivel 3: Sobrevive 60 segundos";

}

void MainWindow::onShowAbout()
{
    QString aboutText =
        "ÚLTIMO BASTIÓN\n\n"
        "Juego de supervivencia con temática medieval\n\n"
        "Niveles:\n"
        "- Nivel 1: Supervivencia en las Murallas\n"
        "- Nivel 2: Esquiva de Barriles\n"
        "- Nivel 3: Carrera de Supervivencia\n\n"
        "Características:\n"
        "- 3 tipos de niveles diferentes\n"
        "- Sistema de niveles y mejoras\n"
        "- Oleadas progresivas de enemigos\n"
        "- Mapa con colisiones\n"
        "- Sistema de audio completo\n\n"
        "Desarrollado con Qt Framework\n"
        "© 2024";

}

void MainWindow::onGamePaused()
{
    statusBar()->showMessage("JUEGO EN PAUSA - Presiona R para reanudar");
}

void MainWindow::onGameResumed()
{
    statusBar()->showMessage("Juego en progreso");
}

void MainWindow::onPlayerLevelUp()
{
    statusBar()->showMessage("¡Nivel subido! Elige una mejora con las teclas 1, 2 o 3");
}

void MainWindow::onGameOver()
{
    statusBar()->showMessage("GAME OVER - Has sido derrotado");

}

void MainWindow::onLevelCompleted()
{
    statusBar()->showMessage("¡VICTORIA! Has completado el nivel");
}
