#ifndef MAINMENU_H
#define MAINMENU_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

class MainMenu : public QWidget
{
    Q_OBJECT

public:
    explicit MainMenu(QWidget *parent = nullptr);

signals:
    void nivel1Seleccionado();
    void nivel2Seleccionado();
    void nivel3Seleccionado();
    void salir();

private slots:
    void onNivel1Clicked();
    void onNivel2Clicked();
    void onNivel3Clicked();
    void onSalirClicked();

private:
    void setupUI();

    QPushButton *btnNivel1;
    QPushButton *btnNivel2;
    QPushButton *btnNivel3;
    QPushButton *btnSalir;
    QLabel *lblTitulo;
};

#endif // MAINMENU_H
