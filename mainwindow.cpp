#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QMessageBox"
#include "QString"
#include "checkprofile.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //sets first-open page
    ui->stackedWidget->setCurrentIndex(1);
    //to be removed
    ui->groupBox_2->hide();

    collapsedWidth = ui->collapseSideBar->width();
    fullWidth = ui->sideBar->maximumWidth();
}

//checking if this works
//yup its working

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_editProfile_clicked()
{
    checkProfile *cp = new checkProfile(this);
    cp->show();
    cp->setModal(true);
}


void MainWindow::on_pushButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}


void MainWindow::on_pushButton_2_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}


void MainWindow::on_collapseSideBar_clicked()
{
    if (ui->sideBar->width() == fullWidth) {
        ui->sideBar->setFixedWidth(collapsedWidth);
        ui->collapseSideBar->setText("->");
    } else {
        ui->sideBar->setFixedWidth(fullWidth);
        ui->collapseSideBar->setText("X");
    }
}

