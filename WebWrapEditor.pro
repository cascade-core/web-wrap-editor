QT += widgets uitools

GIT_VERSION = $$system(git --git-dir $$PWD/.git --work-tree $$PWD describe --always --tags)
DEFINES += GIT_VERSION=\\\"$$GIT_VERSION\\\"

SOURCES = main.cpp mainwindow.cpp
HEADERS = mainwindow.h
RESOURCES = WebWrapEditor.qrc

OBJECTS_DIR=build
MOC_DIR=build
RCC_DIR=build
UI_DIR=build

