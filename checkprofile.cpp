#include "checkprofile.h"
#include "ui_checkprofile.h"
#include "QMessageBox"

checkProfile::checkProfile(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::checkProfile)
{
    ui->setupUi(this);
}

checkProfile::~checkProfile()
{
    delete ui;
}

void checkProfile::on_buttonBox_accepted()
{
    QString username = ui->username->text();
    QString password = ui->password->text();

    if (username == "Mukesh" and password == "123") {
        showChangeProfileOptions = true;
    } else {
        QMessageBox::information(this, "Error!", "Incorrect username or password");
    }
}

