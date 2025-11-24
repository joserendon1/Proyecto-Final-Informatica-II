#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Nivel1;
class Nivel2;
class Nivel3;
class MainMenu;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void mostrarMenuPrincipal();
    void iniciarNivel1();
    void iniciarNivel2();
    void iniciarNivel3();
    void limpiarNivelActual();

    void onNewGame();
    void onPauseGame();
    void onResumeGame();
    void onShowControls();
    void onShowAbout();

    void onGamePaused();
    void onGameResumed();
    void onPlayerLevelUp();
    void onGameOver();
    void onLevelCompleted();

private:
    void setupMenu();
    void setupToolbar();
    void setupStatusBar();
    void connectGameSignals();

private:
    Ui::MainWindow *ui;
    Nivel1 *nivel1;
    Nivel2 *nivel2;
    Nivel3 *nivel3;
    MainMenu *menuPrincipal;
};

#endif // MAINWINDOW_H
