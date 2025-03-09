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


void MainWindow::on_pushButton_clicked()
{

}

