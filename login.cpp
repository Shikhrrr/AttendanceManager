#include "login.h"
#include "ui_login.h"
#include "mainwindow.h"
#include "QMessageBox"

Login::Login(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Login)
{
    ui->setupUi(this);
}

Login::~Login()
{
    delete ui;
}

void Login::on_login_clicked()
{
    username = ui->username->text();
    password = ui->password->text();

    if (username == "Mukesh" and password == "123") {
        QMessageBox::information(this, "Good", "Logged in");
        ui->username->setText("");
        ui->password->setText("");
        MainWindow *mw = new MainWindow();
        mw->show();
        this->hide();

    } else {
        QMessageBox::warning(this, "Bad", "Incorrect username or password");
    }
}

