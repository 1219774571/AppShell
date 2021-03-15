# Created by and for Qt Creator This file was created for editing the project sources only.
# You may attempt to use it for building too, by modifying this file here.

#TARGET = command

QT += core

CONFIG += c++17

CONFIG(debug, debug|release){
outPath = $$PWD/Debug
} else {
outPath = $$PWD/bin
}

MOC_DIR = $${outPath}/moc
RCC_DIR = $${outPath}/rcc
UI_DIR  = $${outPath}/ui
OBJECTS_DIR = $${outPath}/obj
DESTDIR = $${outPath}

#DEFINES =

