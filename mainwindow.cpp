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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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

void MainWindow::on_editProfile_clicked()
{
    // checkProfile *cp = new checkProfile(this);
    // cp->show();
    // cp->setModal(true);
    ui->confirmDetailsGroup->show();
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


void MainWindow::on_gotoProfile_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
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

void MainWindow::on_gotoViewAttendance_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::on_gotoTakeAttendance_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

//for taking attendance


#include <QSqlQueryModel>

void MainWindow::on_takeFetch_clicked()
{
    ui->takeMarkAll->show();
    QString selectedYear = ui->takeYear->currentText();
    QString selectedBranch = ui->takeBranch->currentText();

    QSqlQuery query;
    query.prepare("SELECT roll, name FROM student WHERE year = :year AND branch = :branch");
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
}


// void MainWindow::on_pushButton_clicked()
// {
//         QSqlQuery query;
//         for (int i = 0; i < studentList.size(); i++) {
//             int studentId = studentList[i].id;
//             QString status = (checkboxList[i]->isChecked()) ? "Present" : "Absent";

//             query.prepare("INSERT INTO attendance (student_id, teacher_id, status, date) VALUES (:student_id, :teacher_id, :status, DATE('now'))");
//             query.bindValue(":student_id", studentId);
//             query.bindValue(":teacher_id", loggedInTeacherId);
//             query.bindValue(":status", status);

//             if (!query.exec()) {
//                 qDebug() << "Error inserting attendance:" << query.lastError().text();
//             }
//         }
//         qDebug() << "Attendance submitted successfully!";

// }


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

void MainWindow::on_gotoAddClass_clicked()
{
    ui->stackedWidget->setCurrentIndex(3);
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

