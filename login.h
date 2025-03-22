#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include<QtSql>

namespace Ui {
class Login;
}

class Login : public QWidget
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    QString username = "Mukesh";
    QString password = "123";
    ~Login();

private slots:
    void on_login_clicked();

private:
    Ui::Login *ui;
};

#endif // LOGIN_H
