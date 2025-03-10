#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_editProfile_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_collapseSideBar_clicked();

private:
    Ui::MainWindow *ui;
    int collapsedWidth;
    int fullWidth;
};
#endif // MAINWINDOW_H
