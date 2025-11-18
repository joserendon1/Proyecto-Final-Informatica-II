#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "nivel1.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onGamePaused();
    void onGameResumed();
    void onPlayerLevelUp();
    void onGameOver();
    void onLevelCompleted();

    // Slots para menús
    void onNewGame();
    void onPauseGame();
    void onResumeGame();
    void onShowControls();
    void onShowAbout();

private:
    Ui::MainWindow *ui;
    Nivel1 *nivel1;

    void setupMenu();
    void setupToolbar();
    void setupStatusBar();
    void connectGameSignals();
};

#endif // MAINWINDOW_H
