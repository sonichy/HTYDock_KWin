#-------------------------------------------------
#
# Project created by QtCreator 2020-03-27T11:29:29
#
#-------------------------------------------------

QT       += core gui KWindowSystem x11extras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HTYDock
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    datetimewidget.cpp

HEADERS  += mainwindow.h \
    datetimewidget.h

RESOURCES += \
    res.qrc