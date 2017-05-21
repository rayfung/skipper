#-------------------------------------------------
#
# Project created by QtCreator 2016-01-24T16:35:27
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = skipper
TEMPLATE = app

CONFIG(debug, debug|release): DEFINES += DEBUG_SKIPPER

SOURCES += main.cpp\
        mainwindow.cpp \
    httpserver.cpp \
    httpfileresource.cpp

HEADERS  += mainwindow.h \
    httpserver.h \
    httpfileresource.h

FORMS    += mainwindow.ui

RESOURCES +=
