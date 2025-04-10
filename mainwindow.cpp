#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "deleteattendance.h"
#include "QMessageBox"
#include "QString"
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QFileDialog>
#include <QTextStream>
#include <QSqlQueryModel>
#include <QKeyEvent>
#include "QCheckBox"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->studentTable, &QTableView::clicked, this, &MainWindow::on_studentTable_clicked);

    ui->studentTable->installEventFilter(this);

    ui->viewDateTable->installEventFilter(this);

    // ✅ Set first-open page
    ui->stackedWidget->setCurrentIndex(1);

    // ✅ Hide the option to update credentials
    ui->newDetailsGroup->hide();
    ui->confirmDetailsGroup->hide();
    ui->takeMarkAll->hide();

    collapsedWidth = ui->collapseSideBar->width();
    fullWidth = ui->sideBar->maximumWidth();
    collapsedHeight = ui->collapseSideBar->height();

    // ✅ Connect to the database
    connectToDatabase();

    // 🔥 Fetch and display the current username
    QSqlQuery query;
    if (query.exec("SELECT username FROM admin LIMIT 1")) {  // Assuming only one user in table1 for now
        if (query.next()) {
            QString currentUsername = query.value(0).toString();
            ui->currentName->setText(currentUsername);
        } else {
            qDebug() << "No username found in database";
            ui->currentName->setText("No username found");
        }
    } else {
        qDebug() << "Query failed: " << query.lastError().text();
        ui->currentName->setText("Error fetching username");
    }
    this->setPalette(QPalette());


    ui->downloadCSV->setVisible(false);
    ui->downloadCSV2->setVisible(false);
}

//checking if this works
//yup its working

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectToDatabase() {
    db = QSqlDatabase::addDatabase("QSQLITE");

    QString dbPath = QCoreApplication::applicationDirPath() + "/sample.db";

    db.setDatabaseName(dbPath);

    if (db.open()) {
        qDebug() << "✅ Connected to SQLite!";
        qDebug() << "📂 Database Name: " << db.databaseName();

    } else {
        qDebug() << "❌ Failed to connect:" << db.lastError().text();
        QMessageBox::critical(this, "Database Error", "Failed to connect: " + db.lastError().text());
    }
}

//SWITCH TABS

void MainWindow::on_editProfile_clicked()
{
    ui->confirmDetailsGroup->show();
}
void MainWindow::on_gotoProfile_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}
void MainWindow::on_gotoViewAttendance_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}
void MainWindow::on_gotoTakeAttendance_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_gotoDeleteRecords_clicked()
{
    ui->stackedWidget->setCurrentIndex(4);
}

void MainWindow::on_gotoAddClass_clicked()
{
    ui->stackedWidget->setCurrentIndex(3);
}

void MainWindow::on_collapseSideBar_clicked()
{
    if (ui->sideBar->width() == fullWidth) {
        // ui->sideBar->setFixedWidth(collapsedWidth);
        ui->sideBar->setFixedWidth(collapsedHeight);
        ui->collapseSideBar->setText("->");
    } else {
        ui->sideBar->setFixedWidth(fullWidth);
        ui->collapseSideBar->setText("X");
    }
}

void MainWindow::eraseDetails() {
    ui->username->clear();
    ui->password->clear();
}

void MainWindow::on_cancel_clicked()
{
    ui->confirmDetailsGroup->hide();
    eraseDetails();
}

void MainWindow::on_ok_clicked()
{
    QString enteredUsername = ui->username->text();
    QString enteredPassword = ui->password->text();

    // ✅ Check if the DB connection is open
    if (!QSqlDatabase::database().isOpen()) {
        QMessageBox::critical(this, "Database Error", "Database connection is not open.");
        return;
    }

    // 🔥 Query to verify the username and password
    QSqlQuery query;
    query.prepare("SELECT * FROM admin WHERE username = :username AND password = :password");
    query.bindValue(":username", enteredUsername);
    query.bindValue(":password", enteredPassword);

    if (!query.exec()) {
        qDebug() << "Query Error: " << query.lastError().text();
        QMessageBox::critical(this, "Database Error", "Failed to verify details: " + query.lastError().text());
        return;
    }

    if (query.next()) {  // ✅ Valid username and password
        ui->confirmDetailsGroup->hide();
        QMessageBox::information(this, "Valid", "You can now edit your username and password");
        ui->newDetailsGroup->show();
    } else {             // ❌ Invalid username or password
        QMessageBox::critical(this, "Invalid", "Wrong username or password");
    }

    eraseDetails();  // ✅ Clear input fields
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
    // 🔥 Check if the new password fields match
    if (ui->newPassword->text() != ui->newPasswordConfirmed->text()) {
        QMessageBox::critical(this, "Error", "Passwords don't match");
        return;
    }

    //check empty username
    if (ui->newUserName->text() == "" or ui->newPassword->text() == "") {
        QMessageBox::critical(this, "Error", "Username and password cannot be empty");
        return;
    }

    QString newPassword = ui->newPassword->text();
    QString newUsername = ui->newUserName->text();   // ✅ Get new username

    // ✅ Check if the DB connection is open
    if (!QSqlDatabase::database().isOpen()) {
        QMessageBox::critical(this, "Database Error", "Database connection is not open.");
        return;
    }

    // 🔥 Fetch the current username from the database
    QString currentUsername;
    QSqlQuery fetchQuery;

    if (!fetchQuery.exec("SELECT username FROM admin LIMIT 1")) {
        QMessageBox::critical(this, "Error", "Failed to fetch username: " + fetchQuery.lastError().text());
        return;
    }

    if (fetchQuery.next()) {
        currentUsername = fetchQuery.value(0).toString();
        qDebug() << "Current Username: " << currentUsername;
    } else {
        QMessageBox::warning(this, "Warning", "No username found in the database.");
        return;
    }

    // 🔥 Update both the username and password
    QSqlQuery query;
    query.prepare("UPDATE admin SET username = :newUsername, password = :newPassword WHERE username = :currentUsername");
    query.bindValue(":newUsername", newUsername);
    query.bindValue(":newPassword", newPassword);
    query.bindValue(":currentUsername", currentUsername);

    if (query.exec()) {
        QMessageBox::information(this, "Success", "Username and password updated successfully!");

        // ✅ Clear the input fields
        ui->newUserName->clear();
        ui->newPassword->clear();
        ui->newPasswordConfirmed->clear();
    } else {
        QMessageBox::critical(this, "Error", "Failed to update profile: " + query.lastError().text());
    }
    ui->newDetailsGroup->hide();

    ui->currentName->setText(newUsername);
}
//for taking attendance

void MainWindow::on_takeFetch_clicked()
{
    ui->takeMarkAll->show();
    QString selectedYear = ui->takeYear->currentText();
    QString selectedBranch = ui->takeBranch->currentText();

    QSqlQuery query;
    query.prepare("SELECT roll, name FROM student WHERE year = :year AND branch = :branch ORDER BY roll ASC");
    query.bindValue(":year", selectedYear[0]);
    query.bindValue(":branch", selectedBranch);

    if (!query.exec()) {
        qDebug() << "Error fetching students:" << query.lastError().text();
        QMessageBox::critical(this, "Error", "Failed to fetch students: " + query.lastError().text());
        return;
    }

    // Clear and reset the table
    ui->studentTable->clear();
    ui->studentTable->setRowCount(0);
    ui->studentTable->setColumnCount(3);
    ui->studentTable->setHorizontalHeaderLabels(QStringList() << "ID" << "Name" << "Present");

    int row = 0;
    while (query.next()) {
        int studentId = query.value("roll").toInt();
        QString studentName = query.value("name").toString();

        ui->studentTable->insertRow(row);

        // ID
        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(studentId));
        idItem->setFlags(idItem->flags() ^ Qt::ItemIsEditable);
        ui->studentTable->setItem(row, 0, idItem);

        // Name
        QTableWidgetItem *nameItem = new QTableWidgetItem(studentName);
        nameItem->setFlags(nameItem->flags() ^ Qt::ItemIsEditable);
        ui->studentTable->setItem(row, 1, nameItem);

        // ✅ Styled checkbox as a widget
        QCheckBox *checkbox = new QCheckBox();
        checkbox->setObjectName("styledCheckbox"); // optional if you want to target specifically
        checkbox->setFocusPolicy(Qt::StrongFocus); // ensures it can receive keyboard focus
        checkbox->installEventFilter(this);
        ui->studentTable->setCellWidget(row, 2, checkbox);

        row++;
    }

    ui->studentTable->resizeColumnsToContents();
    ui->studentTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->studentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            QCheckBox *checkBox = qobject_cast<QCheckBox *>(obj);
            if (checkBox) {
                checkBox->toggle(); // switch state
                return true;
            }
        }
    }

    return QMainWindow::eventFilter(obj, event);
}


void MainWindow::on_studentTable_clicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    int row = index.row();
    int checkboxColumn = 2; // "Present" column

    QAbstractItemModel *model = ui->studentTable->model();
    QModelIndex checkboxIndex = model->index(row, checkboxColumn);

    // Toggle checkbox state
    Qt::CheckState currentState = static_cast<Qt::CheckState>(model->data(checkboxIndex, Qt::CheckStateRole).toInt());
    Qt::CheckState newState = (currentState == Qt::Checked) ? Qt::Unchecked : Qt::Checked;

    model->setData(checkboxIndex, newState, Qt::CheckStateRole);
}

void MainWindow::on_takeMarkAll_clicked()
{
    int rowCount = ui->studentTable->rowCount();

    bool markAll = (ui->takeMarkAll->text() == "Mark All");

    ui->takeMarkAll->setText(markAll ? "Remove All" : "Mark All");

    for (int row = 0; row < rowCount; ++row) {
        QWidget *widget = ui->studentTable->cellWidget(row, 2);  // Column 2 has the checkbox
        if (QCheckBox *checkBox = qobject_cast<QCheckBox *>(widget)) {
            checkBox->setChecked(markAll);
        }
    }
}

void MainWindow::on_addSubmit_clicked()
{
    // ✅ Fetch the values from the input fields
    QString roll = ui->addRoll->text().trimmed();
    QString name = ui->addName->text().trimmed();
    QString year = ui->addYear->currentText();
    QString branch = ui->addBranch->currentText();

    // 🚫 Validation: Ensure no empty fields
    if (roll.isEmpty() || name.isEmpty() || year.isEmpty() || branch.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please fill in all fields.");
        return;
    }

    // ✅ Connect to the database
    if (!QSqlDatabase::database().isOpen()) {
        QMessageBox::critical(this, "Database Error", "Database connection is not open.");
        return;
    }

    // 🔥 Insert the data into the `student` table
    QSqlQuery query;
    query.prepare("INSERT INTO student (roll, name, year, branch) VALUES (:roll, :name, :year, :branch)");
    query.bindValue(":roll", roll);
    query.bindValue(":name", name);
    query.bindValue(":year", year);
    query.bindValue(":branch", branch);

    if (!query.exec()) {
        qDebug() << "Insert Error: " << query.lastError().text();
        QMessageBox::critical(this, "Error", "Failed to add student: " + query.lastError().text());
        return;
    }

    // ✅ Clear the fields after successful insertion
    ui->addRoll->clear();
    ui->addName->clear();
    ui->addYear->setCurrentIndex(0);
    ui->addBranch->setCurrentIndex(0);

    QMessageBox::information(this, "Success", "Student added successfully!");
}


void MainWindow::on_signOut_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Sign Out", "Are you sure you want to sign out?", QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QSqlQuery query;
        query.prepare("UPDATE admin SET skipLogin = 0");

        if (!query.exec()) {
            qDebug() << "Failed to reset auto-login:" << query.lastError().text();
            QMessageBox::warning(this, "Error", "Failed to reset auto-login.");
        }
        this->close();
    }
}


// EXCEL IMPORT FEATURE

void MainWindow::on_importCSV_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open CSV File", "", "CSV Files (*.csv)");

    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "Warning", "No file selected.");
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Failed to open the file.");
        return;
    }

    QTextStream in(&file);
    QStringList headers;
    QVector<QStringList> data;

    // Read the headers
    if (!in.atEnd()) {
        headers = in.readLine().split(",");
        ui->tableCSV->setColumnCount(headers.size());
        ui->tableCSV->setHorizontalHeaderLabels(headers);
    }

    // Read the data rows
    while (!in.atEnd()) {
        QStringList row = in.readLine().split(",");
        data.append(row);
    }

    // Set the row count
    ui->tableCSV->setRowCount(data.size());

    // Populate the table
    for (int i = 0; i < data.size(); ++i) {
        const QStringList& row = data[i];
        for (int j = 0; j < row.size(); ++j) {
            QTableWidgetItem *item = new QTableWidgetItem(row[j]);
            ui->tableCSV->setItem(i, j, item);
        }
    }

    file.close();
    QMessageBox::information(this, "Success", "CSV data imported successfully!");
}


void MainWindow::on_submitCSV_clicked()
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        QMessageBox::warning(this, "Database Error", "Database is not connected.");
        return;
    }

    if (ui->tableCSV->rowCount() == 0) {
        QMessageBox::warning(this, "Empty Table", "There is no data to submit.");
        return;
    }

    QSqlQuery query(db);
    bool allRowsInserted = true;

    db.transaction();  // Start a transaction for batch insertion

    for (int row = 0; row < ui->tableCSV->rowCount(); ++row) {
        QStringList values;

        for (int col = 0; col < ui->tableCSV->columnCount(); ++col) {
            values << "'" + ui->tableCSV->item(row, col)->text().trimmed() + "'";
        }

        QString sqlQuery = QString("INSERT INTO student (roll, name, year, branch) VALUES (%1)").arg(values.join(", "));

        qDebug() << "Executing query:" << sqlQuery;  // Log the query

        if (!query.exec(sqlQuery)) {
            qDebug() << "Error inserting row:" << query.lastError().text();
            allRowsInserted = false;
        }
    }

    //chonk

    if (allRowsInserted) {
        db.commit();  // Commit the transaction if all rows are inserted successfully
        QMessageBox::information(this, "Success", "All rows inserted successfully.");
    } else {
        db.rollback();  // Rollback in case of failure
        QMessageBox::warning(this, "Insert Failed", "Failed to insert some rows. Check the console for errors.");
    }
}


void MainWindow::on_cancelCSV_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Clear Table", "Are you sure you want to clear the table?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // Clear the table preview
        ui->tableCSV->clear();
        ui->tableCSV->setRowCount(0);
        ui->tableCSV->setColumnCount(0);

        QMessageBox::information(this, "Cleared", "The table preview has been cleared.");
    }
}


void MainWindow::on_takeSubmit_clicked()
{
    // Get today's date
    QString todayDate = QDate::currentDate().toString("yyyy-MM-dd");

    // Get the selected year and branch
    int year = ui->takeYear->currentText().toInt();
    QString branch = ui->takeBranch->currentText();

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);

    // Step 1: Get date_id for today's date (or insert if not exists)
    query.prepare("SELECT date_id FROM attendance_dates WHERE date = ?");
    query.addBindValue(todayDate);
    if (!query.exec() || !query.next()) {
        // If the date doesn't exist, insert it
        query.prepare("INSERT INTO attendance_dates (date) VALUES (?)");
        query.addBindValue(todayDate);
        if (!query.exec()) {
            qDebug() << "Error inserting attendance date:" << query.lastError();
            return;
        }
        // Retrieve the new date_id
        int newDateId = query.lastInsertId().toInt();
        query.prepare("SELECT date_id FROM attendance_dates WHERE date = ?");
        query.addBindValue(todayDate);
        if (!query.exec() || !query.next()) {
            qDebug() << "Error retrieving new date_id:" << query.lastError();
            return;
        }
    }
    int dateId = query.value(0).toInt();  // Retrieved date_id

    // Step 2: Iterate through studentTable to mark attendance
    for (int row = 0; row < ui->studentTable->model()->rowCount(); ++row) {
        QModelIndex index = ui->studentTable->model()->index(row, 0);
        QString roll = index.data().toString();

        QModelIndex checkboxIndex = ui->studentTable->model()->index(row, 2);
        bool isChecked = checkboxIndex.data(Qt::CheckStateRole) == Qt::Checked;

        QString status = isChecked ? "Present" : "Absent";

        // Step 3: Check if attendance for this student on this date already exists
        query.prepare("SELECT COUNT(*) FROM attendance_records WHERE roll = ? AND date_id = ?");
        query.addBindValue(roll);
        query.addBindValue(dateId);
        if (!query.exec() || !query.next()) {
            qDebug() << "Error checking existing attendance:" << query.lastError();
            continue;
        }

        int count = query.value(0).toInt();
        if (count == 0) {  // Insert only if attendance is not already marked
            query.prepare("INSERT INTO attendance_records (roll, date_id, status) VALUES (?, ?, ?)");
            query.addBindValue(roll);
            query.addBindValue(dateId);
            query.addBindValue(status);

            if (!query.exec()) {
                qDebug() << "Error inserting attendance record:" << query.lastError();
            }
        } else {
            qDebug() << "Attendance already marked for student" << roll << " on " << todayDate;
        }
    }

    QMessageBox::information(this, "Success", "Attendance submitted successfully!");
}


void MainWindow::on_viewDateSubmit_clicked()
{
    QString date = ui->viewDateInput->date().toString("yyyy-MM-dd");
    int year = ui->viewYear2->currentText().toInt();
    QString branch = ui->viewBranch2->currentText();

    QSqlQuery query;
    query.prepare(R"(
        SELECT s.roll, s.name, ar.status
        FROM student s
        JOIN attendance_records ar ON s.roll = ar.roll
        JOIN attendance_dates ad ON ar.date_id = ad.date_id
        WHERE ad.date = ? AND s.year = ? AND s.branch = ?
        ORDER BY s.roll;
    )");

    query.addBindValue(date);
    query.addBindValue(year);
    query.addBindValue(branch);

    if (!query.exec()) {
        qDebug() << "Error fetching attendance records:" << query.lastError();
        return;
    }

    ui->downloadCSV2->setVisible(true);

    // ✅ Set up the QTableWidget
    ui->viewDateTable->clearContents();
    ui->viewDateTable->setRowCount(0);
    ui->viewDateTable->setColumnCount(3);
    ui->viewDateTable->setHorizontalHeaderLabels({"Roll No", "Name", "Present"});

    int row = 0;
    while (query.next()) {
        ui->viewDateTable->insertRow(row);

        // Roll No
        QTableWidgetItem *rollItem = new QTableWidgetItem(query.value(0).toString());
        rollItem->setFlags(rollItem->flags() & ~Qt::ItemIsEditable);
        ui->viewDateTable->setItem(row, 0, rollItem);

        // Name
        QTableWidgetItem *nameItem = new QTableWidgetItem(query.value(1).toString());
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        ui->viewDateTable->setItem(row, 1, nameItem);

        // ✅ Add checkbox
        QCheckBox *checkBox = new QCheckBox(this);
        checkBox->setFocusPolicy(Qt::StrongFocus);
        checkBox->installEventFilter(this);

        QString status = query.value(2).toString().toLower();
        checkBox->setChecked(status == "present");

        ui->viewDateTable->setCellWidget(row, 2, checkBox);

        row++;
    }

    ui->viewDateTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}



void MainWindow::on_viewSearch3_clicked()
{
    QString roll = ui->viewRoll3->text().trimmed();
    if (roll.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter a roll number.");
        return;
    }

    QSqlQuery query;

    // Get student's year and branch
    query.prepare(R"(
        SELECT year, branch FROM student WHERE roll = ?
    )");
    query.addBindValue(roll);

    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, "Not Found", "No student found with this roll number.");
        return;
    }

    int year = query.value(0).toInt();
    QString branch = query.value(1).toString();

    // Get total classes held for that year and branch
    query.prepare(R"(
        SELECT COUNT(DISTINCT ad.date_id)
        FROM attendance_dates ad
        JOIN attendance_records ar ON ad.date_id = ar.date_id
        JOIN student s ON ar.roll = s.roll
        WHERE s.year = ? AND s.branch = ?
    )");
    query.addBindValue(year);
    query.addBindValue(branch);

    int totalClassesHeld = 0;
    if (query.exec() && query.next()) {
        totalClassesHeld = query.value(0).toInt();
    }

    // Get total classes attended by the student
    query.prepare(R"(
        SELECT COUNT(*) FROM attendance_records
        WHERE roll = ? AND status = 'Present'
    )");
    query.addBindValue(roll);

    int totalClassesAttended = 0;
    if (query.exec() && query.next()) {
        totalClassesAttended = query.value(0).toInt();
    }

    // Calculate attendance percentage
    double percentage = (totalClassesHeld == 0) ? 0 : (static_cast<double>(totalClassesAttended) / totalClassesHeld) * 100;

    // Update the QTableWidget (viewStudentSearchTable)
    ui->viewStudentSearchTable->setRowCount(1);  // Set 1 row for the student's summary
    ui->viewStudentSearchTable->setColumnCount(4); // Roll, Total Held, Total Attended, Percentage

    QStringList headers = {"Roll No", "Total Classes Attended", "Total Classes Held", "Attendance %"};
    ui->viewStudentSearchTable->setHorizontalHeaderLabels(headers);

    ui->viewStudentSearchTable->setItem(0, 0, new QTableWidgetItem(roll));
    ui->viewStudentSearchTable->setItem(0, 1, new QTableWidgetItem(QString::number(totalClassesAttended)));
    ui->viewStudentSearchTable->setItem(0, 2, new QTableWidgetItem(QString::number(totalClassesHeld)));
    ui->viewStudentSearchTable->setItem(0, 3, new QTableWidgetItem(QString::number(percentage, 'f', 2) + " %"));

    ui->viewStudentSearchTable->resizeColumnsToContents();
    ui->viewStudentSearchTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->viewStudentSearchTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}


void MainWindow::on_viewStatistics_clicked()
{
    int year = ui->viewYear3->currentText().trimmed().toInt();
    QString branch = ui->viewBranch3->currentText().trimmed();

    // Prepare query to fetch attendance statistics
    QSqlQuery query;
    query.prepare(R"(
    SELECT
        s.roll,
        s.name,
        COUNT(CASE WHEN ar.status = 'Present' THEN 1 END) AS classes_attended,
        (SELECT COUNT(DISTINCT ad.date_id)
         FROM attendance_dates ad
         JOIN attendance_records ar2 ON ad.date_id = ar2.date_id
         JOIN student s2 ON ar2.roll = s2.roll
         WHERE s2.year = s.year AND s2.branch = s.branch) AS total_classes_held,
        ROUND(
            (COUNT(CASE WHEN ar.status = 'Present' THEN 1 END) * 100.0) /
            NULLIF((SELECT COUNT(DISTINCT ad.date_id)
                    FROM attendance_dates ad
                    JOIN attendance_records ar2 ON ad.date_id = ar2.date_id
                    JOIN student s2 ON ar2.roll = s2.roll
                    WHERE s2.year = s.year AND s2.branch = s.branch), 0),
            2
        ) AS percentage
    FROM student s
    LEFT JOIN attendance_records ar ON s.roll = ar.roll
    WHERE s.year = ? AND s.branch = ?
    GROUP BY s.roll, s.name
    ORDER BY s.roll;
)");

    query.addBindValue(year);
    query.addBindValue(branch);

    if (!query.exec()) {
        qDebug() << "Error fetching attendance statistics:" << query.lastError().text();
        return;
    }

    // Set up model for QTableView
    QStandardItemModel *model = new QStandardItemModel(0, 4, this);
    model->setHorizontalHeaderLabels({"Roll Number", "Name", "Classes Attended", "Total Classes Held", "Percentage"});

    // Populate data from the query
    int row = 0;
    while (query.next()) {
        model->insertRow(row);
        model->setItem(row, 0, new QStandardItem(query.value(0).toString())); // Roll Number
        model->setItem(row, 1, new QStandardItem(query.value(1).toString())); // Name
        model->setItem(row, 2, new QStandardItem(query.value(2).toString())); // Classes Attended
        model->setItem(row, 3, new QStandardItem(query.value(3).toString())); // Total Classes Held
        model->setItem(row, 4, new QStandardItem(query.value(4).toString() + "%")); // Percentage
        row++;
    }

    // Set model to QTableView
    ui->viewStatisticsTable->setModel(model);
    ui->viewStatisticsTable->resizeColumnsToContents();
    ui->viewStatisticsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->viewStatisticsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::on_viewSearch4_clicked() {
    int year = ui->viewYear4->currentText().trimmed().toInt();
    QString branch = ui->viewBranch4->currentText().trimmed();
    int minPercentage = ui->viewMoreThan->value();  // MIN should come from "more than"
    int maxPercentage = ui->viewLessThan->value();  // MAX should come from "less than"

    qDebug() << "Searching for: Year=" << year
             << " Branch=" << branch
             << " Percentage between " << minPercentage << " and " << maxPercentage;

    // Step 1: Get all date_ids (classes held) for given year and branch
    QSqlQuery dateQuery;
    dateQuery.prepare(R"(
    SELECT DISTINCT ad.date_id
    FROM attendance_dates ad
    JOIN attendance_records ar ON ad.date_id = ar.date_id
    JOIN student s ON ar.roll = s.roll
    WHERE s.year = ? AND s.branch = ?
)");
    dateQuery.addBindValue(year);
    dateQuery.addBindValue(branch);

    QSet<int> uniqueDates;
    if (dateQuery.exec()) {
        while (dateQuery.next()) {
            uniqueDates.insert(dateQuery.value(0).toInt());
        }
    } else {
        qDebug() << "Error fetching total classes:" << dateQuery.lastError().text();
    }

    int totalClasses = uniqueDates.size();
    qDebug() << "Total classes held:" << totalClasses;

    // Step 2: Get attendance count for each student
    QSqlQuery query;
    query.prepare(R"(
    SELECT s.roll, s.name,
           COUNT(CASE WHEN ar.status = 'Present' THEN 1 END) AS attended
    FROM student s
    LEFT JOIN attendance_records ar ON s.roll = ar.roll
    WHERE s.year = ? AND s.branch = ?
    GROUP BY s.roll, s.name
    ORDER BY s.roll
)");
    query.addBindValue(year);
    query.addBindValue(branch);

    if (!query.exec()) {
        qDebug() << "QUERY ERROR:" << query.lastError().text();
        return;
    }

    // Step 3: Display in QTableWidget
    ui->viewQueryTable->clear();
    ui->viewQueryTable->setRowCount(0);
    ui->viewQueryTable->setColumnCount(5);
    ui->viewQueryTable->setHorizontalHeaderLabels({"Roll No", "Name", "Attended", "Total", "Percentage"});

    int row = 0;
    while (query.next()) {
        int attended = query.value("attended").toInt();
        double percentage = totalClasses == 0 ? 0 : (attended * 100.0) / totalClasses;

        if (percentage >= minPercentage && percentage <= maxPercentage) {
            ui->viewQueryTable->insertRow(row);
            ui->viewQueryTable->setItem(row, 0, new QTableWidgetItem(query.value("roll").toString()));
            ui->viewQueryTable->setItem(row, 1, new QTableWidgetItem(query.value("name").toString()));
            ui->viewQueryTable->setItem(row, 2, new QTableWidgetItem(QString::number(attended)));
            ui->viewQueryTable->setItem(row, 3, new QTableWidgetItem(QString::number(totalClasses)));
            ui->viewQueryTable->setItem(row, 4, new QTableWidgetItem(QString::number(percentage, 'f', 2)));

            row++;
        }
    }

    ui->viewQueryTable->resizeColumnsToContents();
    ui->viewQueryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    if (row == 0) {
        qDebug() << "No records found for given filters.";
    }

}

void MainWindow::on_submitDelete_clicked()
{
    DeleteAttendance del(ui, db);
    del.deleteStudentByRoll();
}

void MainWindow::on_dateDelete_clicked()
{
    DeleteAttendance del(ui, db);
    del.deleteRecordsByDate();
}

void MainWindow::on_editSubmit_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Are you sure?",
                                  "Are you sure you want to edit the attendance for " + ui->viewDateInput->date().toString("yyyy-MM-dd") + "?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) return;

    QString dateStr = ui->viewDateInput->date().toString("yyyy-MM-dd");

    // ✅ Get date_id
    QSqlQuery dateQuery;
    dateQuery.prepare("SELECT date_id FROM attendance_dates WHERE date = ?");
    dateQuery.addBindValue(dateStr);

    if (!dateQuery.exec() || !dateQuery.next()) {
        QMessageBox::warning(this, "Error", "Date not found in the database.");
        return;
    }

    int dateId = dateQuery.value(0).toInt();

    // ✅ Loop through tableWidget rows and update the DB
    QSqlQuery updateQuery;
    QTableWidget *table = ui->viewDateTable;

    for (int row = 0; row < table->rowCount(); ++row) {
        QString roll = table->item(row, 0)->text();

        QWidget *widget = table->cellWidget(row, 2);
        QCheckBox *checkBox = qobject_cast<QCheckBox *>(widget);

        if (!checkBox) continue;

        QString status = checkBox->isChecked() ? "Present" : "Absent";

        updateQuery.prepare(R"(
        UPDATE attendance_records
        SET status = ?
        WHERE roll = ? AND date_id = ?
    )");

        updateQuery.addBindValue(status);
        updateQuery.addBindValue(roll);
        updateQuery.addBindValue(dateId);

        if (!updateQuery.exec()) {
            qDebug() << "Failed to update attendance for" << roll << ":" << updateQuery.lastError().text();
        }
    }

    QMessageBox::information(this, "Success", "Attendance updated successfully.");

    // Optional cleanup (if needed)
    table->clearContents();
    table->setRowCount(0);
}


void MainWindow::on_downloadCSV_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save CSV", "attendance_report.csv", "CSV Files (*.csv)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not open file for writing.");
        return;
    }

    QTextStream out(&file);
    out << "Roll Number,Name,Total Classes Attended,Total Classes Held,Attendance Percentage\n";

    QSqlQuery query;
    query.prepare(R"(
        SELECT s.roll, s.name,
               COUNT(CASE WHEN ar.status = 'Present' THEN 1 END) AS attended,
               COUNT(ad.date_id) AS total_classes,
               (COUNT(CASE WHEN ar.status = 'Present' THEN 1 END) * 100.0 / COUNT(ad.date_id)) AS percentage
        FROM student s
        LEFT JOIN attendance_records ar ON s.roll = ar.roll
        LEFT JOIN attendance_dates ad ON ar.date_id = ad.date_id
        WHERE s.year = ? AND s.branch = ?
        GROUP BY s.roll, s.name
        ORDER BY s.roll;
    )");

    query.addBindValue(ui->viewYear3->currentText().toInt());
    query.addBindValue(ui->viewBranch3->currentText());

    if (!query.exec()) {
        QMessageBox::warning(this, "Error", "Failed to fetch attendance data.");
        file.close();
        return;
    }

    while (query.next()) {
        out << query.value(0).toString() << ","
            << query.value(1).toString() << ","
            << query.value(2).toInt() << ","
            << query.value(3).toInt() << ","
            << query.value(4).toDouble() << "%\n";
    }

    file.close();
    QMessageBox::information(this, "Success", "Attendance report exported successfully!");
    ui->downloadCSV->setVisible(false);
}


void MainWindow::on_downloadCSV2_clicked()
{
    QString date = ui->viewDateInput->date().toString("yyyy-MM-dd");
    int year = ui->viewYear2->currentText().toInt();
    QString branch = ui->viewBranch2->currentText().trimmed();

    // File name format: Attendance_YYYY-MM-DD.csv
    QString fileName = QFileDialog::getSaveFileName(this, "Save Attendance Report",
                                                    QString("Attendance_%1.csv").arg(date),
                                                    "CSV files (*.csv)");
    if (fileName.isEmpty()) {
        return; // User canceled file dialog
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not open file for writing.");
        return;
    }

    QTextStream out(&file);

    // Write the date at the top of the file
    out << "Attendance Report for " << date << "\n\n";

    // Write column headers
    out << "Roll Number,Name,Status\n";

    // Fetch data from database
    QSqlQuery query;
    query.prepare(R"(
        SELECT s.roll, s.name, ar.status
        FROM student s
        JOIN attendance_records ar ON s.roll = ar.roll
        JOIN attendance_dates ad ON ar.date_id = ad.date_id
        WHERE ad.date = ? AND s.year = ? AND s.branch = ?
        ORDER BY s.roll
    )");
    query.addBindValue(date);
    query.addBindValue(year);
    query.addBindValue(branch);

    if (!query.exec()) {
        QMessageBox::warning(this, "Error", "Failed to fetch attendance data: " + query.lastError().text());
        file.close();
        return;
    }

    // Write data rows
    while (query.next()) {
        QString roll = query.value(0).toString();
        QString name = query.value(1).toString();
        QString status = query.value(2).toString();
        out << roll << "," << name << "," << status << "\n";
    }

    file.close();
    ui->downloadCSV2->setVisible(false);
    QMessageBox::information(this, "Success", "Attendance report saved successfully.");
}
