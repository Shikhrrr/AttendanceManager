#ifndef DELETEATTENDANCE_H
#define DELETEATTENDANCE_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDate>

namespace Ui {
class MainWindow; // We're still accessing MainWindow's UI.
}

class DeleteAttendance {
public:
    DeleteAttendance(Ui::MainWindow* ui, QSqlDatabase db);

    void deleteStudentByRoll();
    void deleteRecordsByDate();

private:
    Ui::MainWindow* ui;
    QSqlDatabase db;
};

#endif // DELETEATTENDANCE_H
