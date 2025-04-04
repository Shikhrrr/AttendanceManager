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
        QMessageBox::warning(nullptr, "Error", "Please enter a roll number.");
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

    if (date.isEmpty()) {
        QMessageBox::warning(nullptr, "Error", "Please select a date.");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);

    // Step 1: Get the date_id from attendance_dates
    query.prepare("SELECT date_id FROM attendance_dates WHERE date = ?");
    query.addBindValue(date);

    if (!query.exec() || !query.next()) {
        QMessageBox::warning(nullptr, "Error", "No attendance records found for this date.");
        return;
    }

    int dateId = query.value(0).toInt();

    // Step 2: Confirm deletion
    QMessageBox::StandardButton confirm = QMessageBox::question(
        nullptr, "Confirm Deletion",
        "Are you sure you want to delete all attendance records for " + date + "?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (confirm != QMessageBox::Yes)
        return;

    db.transaction();

    // Step 3: Delete from attendance_records
    QSqlQuery delQuery(db);
    delQuery.prepare("DELETE FROM attendance_records WHERE date_id = ?");
    delQuery.addBindValue(dateId);
    if (!delQuery.exec()) {
        QMessageBox::critical(nullptr, "Error", "Failed to delete attendance records: " + delQuery.lastError().text());
        db.rollback();
        return;
    }

    // Step 4: Delete from attendance_dates
    delQuery.prepare("DELETE FROM attendance_dates WHERE date_id = ?");
    delQuery.addBindValue(dateId);
    if (!delQuery.exec()) {
        QMessageBox::critical(nullptr, "Error", "Failed to delete attendance date: " + delQuery.lastError().text());
        db.rollback();
        return;
    }

    db.commit();

    QMessageBox::information(nullptr, "Success", "Attendance for " + date + " has been deleted.");
}

