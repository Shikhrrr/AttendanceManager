#include "mainwindow.h"
#include "login.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "AttendanceManager_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    Login l;
    l.show();
    // QFile file("C:/Users/shikh/OneDrive/Documents/AttendanceManager/style.qss");
    QFile file("C:/Users/shikh/OneDrive/Documents/AttendanceManager/style.qss");
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "Failed to open style.qss";
    } else {
        QTextStream stream(&file);
        QString styleSheet = stream.readAll();
        qDebug() << "Stylesheet loaded successfully";
        a.setStyleSheet(styleSheet);
        l.setStyleSheet(styleSheet);
    }
    return a.exec();
}
