#include "nivel1.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Nivel1 nivel1;
    nivel1.setWindowTitle("Último Bastión - Nivel 1: Supervivencia en las Murallas");
    nivel1.show();

    return a.exec();
}
