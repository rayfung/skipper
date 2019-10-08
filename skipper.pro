#-------------------------------------------------
#
# Project created by QtCreator 2016-01-24T16:35:27
#
#-------------------------------------------------

QT       += core gui network svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = skipper
TEMPLATE = app

CONFIG(debug, debug|release): DEFINES += DEBUG_SKIPPER

SOURCES += main.cpp\
        mainwindow.cpp \
    httpserver.cpp \
    httpfileresource.cpp \
    qrdialog.cpp \
    qrcode/BitBuffer.cpp \
    qrcode/QrCode.cpp \
    qrcode/QrSegment.cpp

HEADERS  += mainwindow.h \
    httpserver.h \
    httpfileresource.h \
    qrdialog.h \
    qrcode/BitBuffer.hpp \
    qrcode/QrCode.hpp \
    qrcode/QrSegment.hpp

FORMS    += mainwindow.ui \
    qrdialog.ui

RESOURCES +=
