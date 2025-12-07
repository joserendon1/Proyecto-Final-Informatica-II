#include "mainmenu.h"
#include <QApplication>
#include <QFont>
#include <QPalette>
#include <QDebug>

MainMenu::MainMenu(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void MainMenu::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(20);

    lblTitulo = new QLabel("ÚLTIMO BASTIÓN");
    lblTitulo->setAlignment(Qt::AlignCenter);
    QFont titleFont("Arial", 32, QFont::Bold);
    lblTitulo->setFont(titleFont);
    lblTitulo->setStyleSheet("color: #E8C872; background-color: transparent;");

    btnNivel1 = new QPushButton("Nivel 1 - Supervivencia");
    btnNivel2 = new QPushButton("Nivel 2 - Defensa");
    btnNivel3 = new QPushButton("Nivel 3 - Carrera");
    btnSalir = new QPushButton("Salir");

    QString buttonStyle =
        "QPushButton {"
        "    background-color: #4A2C2A;"
        "    color: #E8C872;"
        "    border: 2px solid #E8C872;"
        "    border-radius: 10px;"
        "    padding: 15px;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    min-width: 250px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #5A3C3A;"
        "    border: 2px solid #FFD700;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #3A1C1A;"
        "}";

    btnNivel1->setStyleSheet(buttonStyle);
    btnNivel2->setStyleSheet(buttonStyle);
    btnNivel3->setStyleSheet(buttonStyle);
    btnSalir->setStyleSheet(buttonStyle);

    layout->addWidget(lblTitulo);
    layout->addSpacing(50);
    layout->addWidget(btnNivel1);
    layout->addWidget(btnNivel2);
    layout->addWidget(btnNivel3);
    layout->addSpacing(30);
    layout->addWidget(btnSalir);

    connect(btnNivel1, &QPushButton::clicked, this, &MainMenu::onNivel1Clicked);
    connect(btnNivel2, &QPushButton::clicked, this, &MainMenu::onNivel2Clicked);
    connect(btnNivel3, &QPushButton::clicked, this, &MainMenu::onNivel3Clicked);
    connect(btnSalir, &QPushButton::clicked, this, &MainMenu::onSalirClicked);

    setStyleSheet("background-color: #1E1E1E;");

    qDebug() << "Menú principal configurado";
}

void MainMenu::onNivel1Clicked()
{
    qDebug() << "Nivel 1 seleccionado";
    emit nivel1Seleccionado();
}

void MainMenu::onNivel2Clicked()
{
    qDebug() << "Nivel 2 seleccionado";
    emit nivel2Seleccionado();
}

void MainMenu::onNivel3Clicked()
{
    qDebug() << "Nivel 3 seleccionado";
    emit nivel3Seleccionado();
}

void MainMenu::onSalirClicked()
{
    qDebug() << "Salir seleccionado";
    emit salir();
}
