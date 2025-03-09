#include "checkprofile.h"
#include "ui_checkprofile.h"

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
