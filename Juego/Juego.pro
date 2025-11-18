QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    arma.cpp \
    enemigo.cpp \
    entidad.cpp \
    jugadornivel1.cpp \
    main.cpp \
    mainwindow.cpp \
    mapa.cpp \
    mejora.cpp \
    nivel1.cpp \
    spritemanager.cpp

HEADERS += \
    arma.h \
    enemigo.h \
    entidad.h \
    jugadornivel1.h \
    mainwindow.h \
    mapa.h \
    mejora.h \
    nivel1.h \
    spritemanager.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
