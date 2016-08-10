QT += widgets uitools webkitwidgets script scripttools
CONFIG += debug

DEFAULT_EDITOR_BASE_DIR = "$$system(pwd)/editor/"

GIT_VERSION = $$system(git --git-dir $$PWD/.git --work-tree $$PWD describe --always --tags)
DEFINES += GIT_VERSION=\\\"$$GIT_VERSION\\\"
DEFINES += DEFAULT_EDITOR_BASE_DIR=\\\"$$DEFAULT_EDITOR_BASE_DIR\\\"

SOURCES = main.cpp mainwindow.cpp scriptproxy.cpp settingsdialog.cpp plaintextedit.cpp
HEADERS = mainwindow.h scriptproxy.h settingsdialog.h plaintextedit.h
FORMS = settingsdialog.ui
RESOURCES = WebWrapEditor.qrc

OBJECTS_DIR=build
MOC_DIR=build
RCC_DIR=build
UI_DIR=build

