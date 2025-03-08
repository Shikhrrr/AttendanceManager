#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QMessageBox"
#include "QString"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

//checking if this works
//yup its working

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_login_clicked()
{
    QString username = ui->username->text();
    QString password = ui->password->text();

    if (username == "Mukesh" and password == "123") {
        QMessageBox::information(this, "Good", "Logged in");
    } else {
        QMessageBox::information(this, "Bad", "Incorrect username or password");
    }
}

