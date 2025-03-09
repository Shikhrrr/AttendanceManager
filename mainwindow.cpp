#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QMessageBox"
#include "QString"
#include "profile.h"

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
    username = ui->username->text();
    password = ui->password->text();

    if (username == "Mukesh" and password == "123") {
        QMessageBox::information(this, "Good", "Logged in");
        ui->stackedWidget->setCurrentIndex(1);
    } else {
        QMessageBox::information(this, "Bad", "Incorrect username or password");
    }
}


void MainWindow::on_pushButton_clicked()
{
    if (ui->stackedWidget->currentIndex() == 0) {
        ui->stackedWidget->setCurrentIndex(1);
    }
    else {
        ui->stackedWidget->setCurrentIndex(0);
    }
}

