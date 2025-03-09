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
    bool showChangeProfileOptions = false;
    ~checkProfile();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::checkProfile *ui;
};

#endif // CHECKPROFILE_H
