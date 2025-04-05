QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Deployment Rules
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Source and header files
SOURCES += \
    # checkprofile.cpp \
    deleteattendance.cpp \
    login.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    # checkprofile.h \
    deleteattendance.h \
    login.h \
    mainwindow.h

FORMS += \
    # checkprofile.ui \
    login.ui \
    mainwindow.ui

# # Translations
# TRANSLATIONS += \
#     AttendanceManager_en_AS.ts

# CONFIG += lrelease
# CONFIG += embed_translations

# Output directories
OBJECTS_DIR = $$PWD/build/debug
MOC_DIR = $$PWD/build/moc
RCC_DIR = $$PWD/build/rcc
UI_DIR = $$PWD/build/ui

DISTFILES += \
    style.qss
