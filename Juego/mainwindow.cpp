#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QMessageBox>
#include <QDebug>
#include <QVBoxLayout>
#include <QScreen>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , nivel1(nullptr)
{
    ui->setupUi(this);

    setWindowTitle("Último Bastión");

    int anchoVentana = 1024;
    int altoVentana = 768;

    setFixedSize(anchoVentana, altoVentana);

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - anchoVentana) / 2;
    int y = (screenGeometry.height() - altoVentana) / 2;
    move(x, y);

    setFocusPolicy(Qt::StrongFocus);

    setupMenu();
    setupToolbar();
    setupStatusBar();

    nivel1 = new Nivel1(this);
    setCentralWidget(nivel1);

    connectGameSignals();

    qDebug() << "Ventana configurada - 1024x768 fija";

    statusBar()->showMessage("Juego listo - Usa WASD para moverte, P para pausar");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupMenu()
{
    QMenu *fileMenu = menuBar()->addMenu("&Archivo");

    QAction *newGameAction = new QAction("&Nuevo Juego", this);
    newGameAction->setShortcut(QKeySequence::New);
    QAction *exitAction = new QAction("&Salir", this);
    exitAction->setShortcut(QKeySequence::Quit);

    fileMenu->addAction(newGameAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    connect(newGameAction, &QAction::triggered, this, &MainWindow::onNewGame);
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

    QMenu *helpMenu = menuBar()->addMenu("&Ayuda");

    QAction *controlsAction = new QAction("&Controles", this);
    QAction *aboutAction = new QAction("&Acerca de", this);

    helpMenu->addAction(controlsAction);
    helpMenu->addAction(aboutAction);

    connect(controlsAction, &QAction::triggered, this, &MainWindow::onShowControls);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onShowAbout);
}

void MainWindow::setupToolbar()
{
    QToolBar *gameToolbar = addToolBar("Juego");
    gameToolbar->setMovable(false);

    QAction *newGameAction = new QAction("Nuevo Juego", this);
    QAction *pauseAction = new QAction("Pausar", this);
    QAction *resumeAction = new QAction("Reanudar", this);

    gameToolbar->addAction(newGameAction);
    gameToolbar->addSeparator();
    gameToolbar->addAction(pauseAction);
    gameToolbar->addAction(resumeAction);

    connect(newGameAction, &QAction::triggered, this, &MainWindow::onNewGame);
    connect(pauseAction, &QAction::triggered, this, &MainWindow::onPauseGame);
    connect(resumeAction, &QAction::triggered, this, &MainWindow::onResumeGame);
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
}

void MainWindow::onNewGame()
{
    if (nivel1) {
        // Reiniciar el nivel
        delete nivel1;
        nivel1 = new Nivel1(this);
        setCentralWidget(nivel1);
        connectGameSignals();

        statusBar()->showMessage("Nuevo juego iniciado");
        QMessageBox::information(this, "Nuevo Juego",
                                 "¡Nuevo juego iniciado! Sobrevive 2 minutos contra las oleadas de enemigos.");
    }
}

void MainWindow::onPauseGame()
{
    if (nivel1) {
        nivel1->pausarNivel();
    }
}

void MainWindow::onResumeGame()
{
    if (nivel1) {
        nivel1->reanudarNivel();
    }
}

void MainWindow::onShowControls()
{
    QString controles =
        "CONTROLES DEL JUEGO:\n\n"
        "WASD - Movimiento del jugador\n"
        "P - Pausar juego\n"
        "R - Reanudar juego\n"
        "1, 2, 3 - Seleccionar mejoras cuando subas de nivel\n\n"
        "OBJETIVO:\n"
        "Sobrevive 2 minutos contra oleadas de enemigos\n"
        "Sube de nivel y elige nuevas armas\n"
        "Recolecta experiencia derrotando enemigos";

    QMessageBox::information(this, "Controles del Juego", controles);
}

void MainWindow::onShowAbout()
{
    QString aboutText =
        "ÚLTIMO BASTIÓN\n\n"
        "Juego de supervivencia con temática medieval\n"
        "Nivel 1: Supervivencia en las Murallas\n\n"
        "Características:\n"
        "- 4 tipos de armas diferentes\n"
        "- Sistema de niveles y mejoras\n"
        "- Oleadas progresivas de enemigos\n"
        "- Mapa con colisiones\n\n"
        "Desarrollado con Qt Framework\n"
        "© 2024";

    QMessageBox::about(this, "Acerca de Último Bastión", aboutText);
}

void MainWindow::onGamePaused()
{
    statusBar()->showMessage("JUEGO EN PAUSA - Presiona R para reanudar");
}

void MainWindow::onGameResumed()
{
    statusBar()->showMessage("Juego en progreso - Sobrevive 2 minutos!");
}

void MainWindow::onPlayerLevelUp()
{
    statusBar()->showMessage("¡Nivel subido! Elige una mejora con las teclas 1, 2 o 3");
}

void MainWindow::onGameOver()
{
    statusBar()->showMessage("GAME OVER - Has sido derrotado");

    QMessageBox gameOverMsg;
    gameOverMsg.setWindowTitle("Game Over");
    gameOverMsg.setText("¡Has sido derrotado!");
    gameOverMsg.setInformativeText("¿Quieres intentarlo de nuevo?");
    gameOverMsg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    gameOverMsg.setDefaultButton(QMessageBox::Yes);

    int respuesta = gameOverMsg.exec();
    if (respuesta == QMessageBox::Yes) {
        onNewGame();
    }
}

void MainWindow::onLevelCompleted()
{
    statusBar()->showMessage("¡VICTORIA! Has completado el nivel");

    QMessageBox victoryMsg;
    victoryMsg.setWindowTitle("¡Victoria!");
    victoryMsg.setText("¡Felicidades! Has sobrevivido 2 minutos.");
    victoryMsg.setInformativeText("¿Quieres jugar de nuevo?");
    victoryMsg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    victoryMsg.setDefaultButton(QMessageBox::Yes);

    int respuesta = victoryMsg.exec();
    if (respuesta == QMessageBox::Yes) {
        onNewGame();
    }
}
