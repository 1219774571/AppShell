QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(../command.pri)

SOURCES += \
    AppConsole.cpp \
    ArgsFormatData.cpp \
    Tool.cpp \
    main.cpp \
    mainwindow.cpp \
    xxtea/xxtea.c \
    xxteaInteface.cpp

HEADERS += \
    AppConsole.h \
    ArgsFormatData.h \
    Tool.h \
    mainwindow.h \
    version.h \
    xxtea/xxtea.h \
    xxteaInteface.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    images.qrc


RC_ICONS = "ico.ico"
