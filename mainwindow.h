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

    void on_collapseSideBar_clicked();

    void on_gotoProfile_clicked();

    void on_cancel_clicked();

    void on_ok_clicked();

    void on_cancelNew_clicked();

    void on_reset_clicked();

    void eraseDetails();

    void on_gotoViewAttendance_clicked();

    void on_gotoTakeAttendance_clicked();

    void on_takeFetch_clicked();

    // void on_pushButton_clicked();

    void on_takeMarkAll_clicked();

private:
    Ui::MainWindow *ui;
    int collapsedWidth;
    int fullWidth;
    QString username;
    QString password;
    void connectToDatabase();
};
#endif // MAINWINDOW_H
