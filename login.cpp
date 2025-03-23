#include "login.h"
#include "ui_login.h"
#include "mainwindow.h"
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

Login::Login(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Login)
{
    ui->setupUi(this);
    connectToDatabase();
}

Login::~Login()
{
    delete ui;
}

void Login::connectToDatabase() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

    // 🔥 Check and print the actual database path
    QString dbPath = QCoreApplication::applicationDirPath() + "/sample.db";
    // qDebug() << "Database Path:" << dbPath;

    // ✅ Use absolute path for testing (remove later for portability)
    // db.setDatabaseName("C:/Users/shikh/OneDrive/Documents/AttendanceManager/sample.db");

    // ✅ Use relative path (after confirming it works)
    db.setDatabaseName(dbPath);

    if (db.open()) {
        qDebug() << "✅ Connected to SQLite!";
        qDebug() << "📂 Database Name: " << db.databaseName();

        // // 🔥 Verify tables
        // QSqlQuery query("SELECT name FROM sqlite_master WHERE type='table'");

        // if (!query.exec()) {
        //     QMessageBox::critical(this, "Error", "Failed to retrieve table names: " + query.lastError().text());
        //     qDebug() << "Query Error:" << query.lastError().text();
        //     return;
        // }

        // QString tableList;
        // while (query.next()) {
        //     tableList += query.value(0).toString() + "\n";
        // }

        // if (tableList.isEmpty()) {
        //     tableList = "No tables found!";
        // }

        // QMessageBox::information(this, "Tables in Database", tableList);
        // qDebug() << "Tables: " << tableList;

    } else {
        ui->label->setText("could not connect");
        qDebug() << "❌ Failed to connect:" << db.lastError().text();
        QMessageBox::critical(this, "Database Error", "Failed to connect: " + db.lastError().text());
    }
}

void Login::on_login_clicked()
{
    // Get username and password from input fields
    username = ui->username->text();
    password = ui->password->text();

    // ✅ Verify database connection
    if (!QSqlDatabase::database().isOpen()) {
        QMessageBox::critical(this, "Database Error", "Database connection is not open.");
        return;
    }

    // 🔥 Query to validate credentials
    QSqlQuery query;

    // Prepare query safely
    if (!query.prepare("SELECT * FROM table1 WHERE username = :username AND password = :password")) {
        qDebug() << "Query preparation failed: " << query.lastError().text();
        return;
    }

    query.bindValue(":username", username);
    query.bindValue(":password", password);

    // ✅ Execute the query ONCE
    if (!query.exec()) {
        qDebug() << "Query Error: " << query.lastError().text();
        QMessageBox::warning(this, "Database Error", "Query failed: " + query.lastError().text());
        return;
    }

    if (query.next()) {  // ✅ Check if there are results
        // ✅ Valid login
        QMessageBox::information(this, "Success", "Logged in successfully!");

        // Clear input fields
        ui->username->clear();
        ui->password->clear();

        // Open MainWindow
        MainWindow *mw = new MainWindow();
        mw->show();
        this->hide();

    } else {
        // ❌ Invalid login
        QMessageBox::warning(this, "Invalid", "Incorrect username or password");
    }
}
