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

    //hides the option to update credentials **CRITICAL
    ui->newDetailsGroup->hide();
    ui->confirmDetailsGroup->hide();

    collapsedWidth = ui->collapseSideBar->width();
    fullWidth = ui->sideBar->maximumWidth();
    username = "Mukesh";
    password = "123";

    ui->currentName->setText(username);
}

//checking if this works
//yup its working

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_editProfile_clicked()
{
    // checkProfile *cp = new checkProfile(this);
    // cp->show();
    // cp->setModal(true);
    ui->confirmDetailsGroup->show();
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


void MainWindow::on_gotoProfile_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::eraseDetails() {
    ui->username->setText("");
    ui->password->setText("");
}

void MainWindow::on_cancel_clicked()
{
    ui->confirmDetailsGroup->hide();
    eraseDetails();
}


void MainWindow::on_ok_clicked()
{
    if (ui->username->text() == "Mukesh" and ui->password->text() == "123") {
        ui->confirmDetailsGroup->hide();
        QMessageBox::information(this, "Valid", "You can now edit your username and password");
        ui->newDetailsGroup->show();
    } else {
        QMessageBox::critical(this, "X", "Wrong username or password");
    }
    eraseDetails();
}


void MainWindow::on_cancelNew_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "?", "Are you sure you want to cancel?", QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        ui->newDetailsGroup->hide();
    }
}


void MainWindow::on_reset_clicked()
{
    if (ui->newPassword->text() == ui->newPasswordConfirmed->text()) {
        //change username and password
        username = ui->newUserName->text();
        password = ui->newPassword->text();
        QMessageBox::information(this, "Success", "Username and password updated successfully");
        ui->newDetailsGroup->hide();
        ui->currentName->setText(username);
    } else {
        QMessageBox::critical(this, "X", "Passwords don't match");
    }
}

