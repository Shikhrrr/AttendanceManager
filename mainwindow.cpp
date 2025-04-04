#include "mainwindow.h"
#include "ui_mainwindow.h"
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
    // Apply dynamic stretching to keep proportions on window resize
    // ui->viewAttendanceLayout->setRowStretch(0, 1);
    // ui->viewAttendanceLayout->setRowStretch(1, 5);
    // ui->viewAttendanceLayout->setRowStretch(2, 1);

    // ui->viewAttendanceLayout->setColumnStretch(0, 1);
    // ui->viewAttendanceLayout->setColumnStretch(1, 1);
    // ui->viewAttendanceLayout->setColumnStretch(2, 1);

    // Ensure the parent widget is using this layout
    // ui->stackedWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // ui->viewPageTab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // ui->importCSV->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

//checking if this works
//yup its working

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectToDatabase() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

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
        ui->sideBar->setFixedWidth(collapsedWidth);
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

    // ✅ Create a model for the table
    QStandardItemModel *model = new QStandardItemModel(this);

    // ✅ Set table headers
    model->setHorizontalHeaderLabels(QStringList() << "ID" << "Name" << "Present");

    int row = 0;
    while (query.next()) {
        int studentId = query.value("roll").toInt();
        QString studentName = query.value("name").toString();

        // ✅ Add ID and Name as table rows
        QStandardItem *idItem = new QStandardItem(QString::number(studentId));
        QStandardItem *nameItem = new QStandardItem(studentName);

        // ✅ Add checkbox item
        QStandardItem *checkboxItem = new QStandardItem();
        checkboxItem->setCheckable(true);
        checkboxItem->setCheckState(Qt::Unchecked);  // Initially unchecked

        // ✅ Add items to the model
        model->setItem(row, 0, idItem);
        model->setItem(row, 1, nameItem);
        model->setItem(row, 2, checkboxItem);

        row++;
    }

    // ✅ Set the model to the `QTableView`
    ui->studentTable->setModel(model);
    ui->studentTable->resizeColumnsToContents();
    ui->studentTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->studentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if ((obj == ui->studentTable || obj == ui->viewDateTable) && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);  // ✅ Safe cast

        if (keyEvent && (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)) {
            QTableView *tableView = qobject_cast<QTableView *>(obj);
            if (!tableView) return false;

            QModelIndex currentIndex = tableView->currentIndex();
            if (!currentIndex.isValid()) return false;

            int row = currentIndex.row();
            int checkboxColumn = 2; // "Present" column

            QAbstractItemModel *model = tableView->model();
            QModelIndex checkboxIndex = model->index(row, checkboxColumn);

            // ✅ Toggle checkbox state
            Qt::CheckState currentState = static_cast<Qt::CheckState>(model->data(checkboxIndex, Qt::CheckStateRole).toInt());
            Qt::CheckState newState = (currentState == Qt::Checked) ? Qt::Unchecked : Qt::Checked;

            model->setData(checkboxIndex, newState, Qt::CheckStateRole);

            return true; // ✅ Event handled
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
    // ✅ Get the model from the tableView
    QAbstractItemModel *model = ui->studentTable->model();

    if (!model) {
        qDebug() << "No model found!";
        return;
    }

    // ✅ Loop through all rows and set the checkbox checked
    if (ui->takeMarkAll->text() == "Mark All") {
        ui->takeMarkAll->setText("Remove All");
        for (int row = 0; row < model->rowCount(); ++row) {
            QModelIndex index = model->index(row, 2);  // Column 2: Checkbox column
            model->setData(index, Qt::Checked, Qt::CheckStateRole);
        }
    } else {
        ui->takeMarkAll->setText("Mark All");
        for (int row = 0; row < model->rowCount(); ++row) {
            QModelIndex index = model->index(row, 2);  // Column 2: Checkbox column
            model->setData(index, Qt::Unchecked, Qt::CheckStateRole);
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
    QString date = ui->dateEdit->date().toString("yyyy-MM-dd");  // Selected date
    int year = ui->viewYear2->currentText().toInt();         // Selected year
    QString branch = ui->viewBranch2->currentText();         // Selected branch

    // ✅ Step 1: Prepare SQL Query
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

    // ✅ Step 2: Execute Query
    if (!query.exec()) {
        qDebug() << "Error fetching attendance records:" << query.lastError();
        return;
    }

    // ✅ Step 3: Create Table Model
    QStandardItemModel *model = new QStandardItemModel(this);
    model->setColumnCount(3);
    model->setHeaderData(0, Qt::Horizontal, "Roll No");
    model->setHeaderData(1, Qt::Horizontal, "Name");
    model->setHeaderData(2, Qt::Horizontal, "Present");  // Renamed column to "Present"

    // ✅ Step 4: Populate Table with Checkboxes
    int row = 0;
    while (query.next()) {
        model->insertRow(row);

        // Roll No
        QStandardItem *rollItem = new QStandardItem(query.value(0).toString());
        rollItem->setFlags(rollItem->flags() & ~Qt::ItemIsEditable); // ❌ Make non-editable

        // Name
        QStandardItem *nameItem = new QStandardItem(query.value(1).toString());
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable); // ❌ Make non-editable

        // Checkbox for Attendance Status
        QStandardItem *statusItem = new QStandardItem();
        statusItem->setCheckable(true);

        // ✅ Set checkbox state (Present = ✅ Checked, Absent = ❌ Unchecked)
        if (query.value(2).toString().toLower() == "present") {
            statusItem->setCheckState(Qt::Checked);
        } else {
            statusItem->setCheckState(Qt::Unchecked);
        }

        // ✅ Add items to model
        model->setItem(row, 0, rollItem);
        model->setItem(row, 1, nameItem);
        model->setItem(row, 2, statusItem);

        row++;
    }

    // ✅ Step 5: Set the Model to the TableView
    ui->viewDateTable->setModel(model);
    ui->viewDateTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->viewDateTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // ❌ Disable text editing
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



//this needs to be fixed


void MainWindow::on_viewSearch4_clicked() {
    // 1. Get UI inputs
    int year = ui->viewYear4->currentText().trimmed().toInt();
    QString branch = ui->viewBranch4->currentText().trimmed();
    int minPercentage = ui->viewLessThan->value();
    int maxPercentage = ui->viewMoreThan->value();

    qDebug() << "Searching for: Year=" << year
             << "Branch=" << branch
             << "Percentage=" << minPercentage << "-" << maxPercentage;

    // 2. Execute Query
    QSqlQuery query;
    query.prepare(R"(
        WITH total_classes AS (
            SELECT COUNT(DISTINCT ad.date_id) AS total_classes_held
            FROM attendance_dates ad
            JOIN attendance_records ar ON ad.date_id = ar.date_id
            JOIN student s ON ar.roll = s.roll
            WHERE s.year = ? AND s.branch = ?
        )
        SELECT
            s.roll,
            s.name,
            COUNT(CASE WHEN ar.status = 'Present' THEN 1 END) AS attended,
            (SELECT total_classes_held FROM total_classes) AS total,
            ROUND(
                (COUNT(CASE WHEN ar.status = 'Present' THEN 1 END) * 100.0) /
                NULLIF((SELECT total_classes_held FROM total_classes), 0),
                2
            ) AS percentage
        FROM student s
        LEFT JOIN attendance_records ar ON s.roll = ar.roll
        WHERE s.year = ? AND s.branch = ?
        GROUP BY s.roll, s.name
        HAVING (percentage >= ? AND percentage <= ?) OR (percentage IS NULL)
    )");

    query.addBindValue(year);
    query.addBindValue(branch);
    query.addBindValue(year);
    query.addBindValue(branch);
    query.addBindValue(minPercentage);
    query.addBindValue(maxPercentage);

    if (!query.exec()) {
        qDebug() << "QUERY ERROR:" << query.lastError().text();
        return;
    }

    // 3. Check for results
    if (!query.next()) {
        qDebug() << "NO RECORDS FOUND. Check filters/database.";
        ui->viewQueryTable->setRowCount(0); // Clear table
        return;
    }

    // 4. Populate table
    ui->viewQueryTable->setRowCount(0);
    ui->viewQueryTable->setColumnCount(5);
    ui->viewQueryTable->setHorizontalHeaderLabels({"Roll", "Name", "Attended", "Total", "%"});

    do {
        int row = ui->viewQueryTable->rowCount();
        ui->viewQueryTable->insertRow(row);
        ui->viewQueryTable->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        ui->viewQueryTable->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        ui->viewQueryTable->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
        ui->viewQueryTable->setItem(row, 3, new QTableWidgetItem(query.value(3).toString()));
        ui->viewQueryTable->setItem(row, 4, new QTableWidgetItem(
                                                query.value(4).isNull() ? "N/A" : query.value(4).toString() + "%"
                                                ));
    } while (query.next());

    ui->viewQueryTable->resizeColumnsToContents();
}


void MainWindow::on_submitDelete_clicked()
{
    QString roll = ui->deleteRollInput->text().trimmed(); // Get roll number from input field

    if (roll.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a roll number.");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);

    // Confirm before deletion
    QMessageBox::StandardButton confirm;
    confirm = QMessageBox::question(this, "Confirm Deletion",
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
        QMessageBox::critical(this, "Error", "Failed to delete attendance records: " + query.lastError().text());
        db.rollback();
        return;
    }

    // Step 2: Delete from student table
    query.prepare("DELETE FROM student WHERE roll = ?");
    query.addBindValue(roll);

    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "Failed to delete student record: " + query.lastError().text());
        db.rollback();
        return;
    }

    // Commit transaction
    db.commit();
    QMessageBox::information(this, "Success", "Student deleted successfully.");
}

void MainWindow::on_dateDelete_clicked()
{
    QString date = ui->deleteDateInput->date().toString("yyyy-MM-dd");

    if (date.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select a date.");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);

    // Step 1: Get the date_id from attendance_dates
    query.prepare("SELECT date_id FROM attendance_dates WHERE date = ?");
    query.addBindValue(date);

    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, "Error", "No attendance records found for this date.");
        return;
    }

    int dateId = query.value(0).toInt();

    // Step 2: Confirm deletion
    QMessageBox::StandardButton confirm = QMessageBox::question(
        this, "Confirm Deletion",
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
        QMessageBox::critical(this, "Error", "Failed to delete attendance records: " + delQuery.lastError().text());
        db.rollback();
        return;
    }

    // Step 4: Delete from attendance_dates
    delQuery.prepare("DELETE FROM attendance_dates WHERE date_id = ?");
    delQuery.addBindValue(dateId);
    if (!delQuery.exec()) {
        QMessageBox::critical(this, "Error", "Failed to delete attendance date: " + delQuery.lastError().text());
        db.rollback();
        return;
    }

    db.commit();

    QMessageBox::information(this, "Success", "Attendance for " + date + " has been deleted.");
}

