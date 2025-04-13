#include "deleteattendance.h"
#include "ui_mainwindow.h"
#include "QMessageBox"
#include <QSqlError>
#include <QSqlQuery>

DeleteAttendance::DeleteAttendance(Ui::MainWindow* ui, QSqlDatabase db)
    : ui(ui), db(db) {}

void DeleteAttendance::deleteStudentByRoll() {
    QString roll = ui->deleteRollInput->text().trimmed(); // Get roll number from input field

    if (roll.isEmpty()) {
        QMessageBox::critical(nullptr, "Error", "Please enter a roll number.");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);

    // Confirm before deletion
    QMessageBox::StandardButton confirm;
    confirm = QMessageBox::question(nullptr, "Confirm Deletion",
                                    "Are you sure you want to delete this student and all related records?",
                                    QMessageBox::Yes | QMessageBox::No);

    if (confirm == QMessageBox::No) {
        return; // Do nothing if user cancels
    }

    // Start transaction
    db.transaction();

    // Step 1: Delete from attendance_records
    query.prepare("DELETE FROM attendance_records WHERE roll = ?");
    query.addBindValue(roll);

    if (!query.exec()) {
        QMessageBox::critical(nullptr, "Error", "Failed to delete attendance records: " + query.lastError().text());
        db.rollback();
        return;
    }

    // Step 2: Delete from student table
    query.prepare("DELETE FROM student WHERE roll = ?");
    query.addBindValue(roll);

    if (!query.exec()) {
        QMessageBox::critical(nullptr, "Error", "Failed to delete student record: " + query.lastError().text());
        db.rollback();
        return;
    }

    // Commit transaction
    db.commit();
    QMessageBox::information(nullptr, "Success", "Student deleted successfully.");
}

//---------------------------------------------

void DeleteAttendance::deleteRecordsByDate() {
    QString date = ui->deleteDateInput->date().toString("yyyy-MM-dd");
    int year = ui->deleteYear->currentText().toInt();
    QString branch = ui->deleteBranch->currentText().trimmed();

    if (date.isEmpty()) {
        QMessageBox::critical(nullptr, "Error", "Please select a date.");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);

    // ✅ Step 1: Get the date_id for the selected year, branch, and date
    query.prepare(R"(
        SELECT ad.date_id
        FROM attendance_dates ad
        JOIN attendance_records ar ON ad.date_id = ar.date_id
        JOIN student s ON ar.roll = s.roll
        WHERE ad.date = ? AND s.year = ? AND s.branch = ?
        LIMIT 1
    )");

    query.addBindValue(date);
    query.addBindValue(year);
    query.addBindValue(branch);

    if (!query.exec() || !query.next()) {
        QMessageBox::warning(nullptr, "Error", "No attendance records found for nullptr date, year, and branch.");
        return;
    }

    int dateId = query.value(0).toInt();

    // ✅ Step 2: Confirm Deletion
    QMessageBox::StandardButton confirm = QMessageBox::question(
        nullptr, "Confirm Deletion",
        "Are you sure you want to delete all attendance records for " + date +
            " in " + QString::number(year) + " " + branch + "?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (confirm != QMessageBox::Yes)
        return;

    db.transaction();

    // ✅ Step 3: Delete only relevant records from attendance_records
    QSqlQuery delQuery(db);
    delQuery.prepare(R"(
        DELETE FROM attendance_records
        WHERE date_id = ? AND roll IN (
            SELECT roll FROM student WHERE year = ? AND branch = ?
        )
    )");
    delQuery.addBindValue(dateId);
    delQuery.addBindValue(year);
    delQuery.addBindValue(branch);

    if (!delQuery.exec()) {
        QMessageBox::critical(nullptr, "Error", "Failed to delete attendance records: " + delQuery.lastError().text());
        db.rollback();
        return;
    }

    // ✅ Step 4: Delete date only if no other records refer to it
    delQuery.prepare("SELECT COUNT(*) FROM attendance_records WHERE date_id = ?");
    delQuery.addBindValue(dateId);
    if (!delQuery.exec() || !delQuery.next()) {
        db.rollback();
        QMessageBox::critical(nullptr, "Error", "Error checking remaining records.");
        return;
    }

    if (delQuery.value(0).toInt() == 0) {
        QSqlQuery dateDeleteQuery(db);
        dateDeleteQuery.prepare("DELETE FROM attendance_dates WHERE date_id = ?");
        dateDeleteQuery.addBindValue(dateId);

        if (!dateDeleteQuery.exec()) {
            QMessageBox::critical(nullptr, "Error", "Failed to delete date entry: " + dateDeleteQuery.lastError().text());
            db.rollback();
            return;
        }
    }

    db.commit();
    QMessageBox::information(nullptr, "Success", "Attendance for " + date + " has been deleted.");
}
