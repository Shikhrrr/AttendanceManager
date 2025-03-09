#ifndef CHECKPROFILE_H
#define CHECKPROFILE_H

#include <QDialog>

namespace Ui {
class checkProfile;
}

class checkProfile : public QDialog
{
    Q_OBJECT

public:
    explicit checkProfile(QWidget *parent = nullptr);
    ~checkProfile();

private:
    Ui::checkProfile *ui;
};

#endif // CHECKPROFILE_H
