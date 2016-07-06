#-------------------------------------------------
#
# Project created by QtCreator 2016-06-27T00:56:00
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = parser_test
TEMPLATE = app


SOURCES += main.cpp\
        parsertest.cpp \
    parser.cpp \
    search_functions.cpp

HEADERS  += parsertest.h \
    parser.h

FORMS    += parsertest.ui

RESOURCES += \
    parsertest.qrc
