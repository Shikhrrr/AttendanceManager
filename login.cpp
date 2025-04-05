#include "login.h"
#include "ui_login.h"
#include "mainwindow.h"
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QTimer>

Login::Login(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Login)
{
    ui->setupUi(this);
    connectToDatabase();
    checkAutoLogin();  // 🔥 Check if skipLogin is enabled on startup
}

Login::~Login()
{
    delete ui;
}

void Login::connectToDatabase() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

    // ✅ Use relative path for portability
    QString dbPath = QCoreApplication::applicationDirPath() + "/sample.db";
    db.setDatabaseName(dbPath);

    if (db.open()) {
        qDebug() << "✅ Connected to SQLite!";
    } else {
        ui->label->setText("Could not connect");
        qDebug() << "❌ Failed to connect:" << db.lastError().text();
        QMessageBox::critical(this, "Database Error", "Failed to connect: " + db.lastError().text());
    }
}

void Login::checkAutoLogin() {
    QSqlQuery query;
    query.prepare("SELECT skipLogin FROM admin LIMIT 1");

    if (!query.exec()) {
        qDebug() << "Auto-login check failed: " << query.lastError().text();
        return;
    }

    if (query.next()) {
        int skipLogin = query.value(0).toInt();
        if (skipLogin == 1) {
            qDebug() << "✅ Auto-logging in...";

            // ✅ Use QTimer to avoid closing issues during constructor execution
            QTimer::singleShot(0, this, [=]() {
                MainWindow *mw = new MainWindow();
                mw->show();
                this->close();
            });
        }
    }
}

void Login::on_login_clicked()
{
    // Get username and password from input fields
    QString username = ui->username->text().trimmed();
    QString password = ui->password->text().trimmed();

    // ✅ Verify database connection
    if (!QSqlDatabase::database().isOpen()) {
        QMessageBox::critical(this, "Database Error", "Database connection is not open.");
        return;
    }

    // 🔥 Query to validate credentials
    QSqlQuery query;
    query.prepare("SELECT * FROM admin WHERE username = :username AND password = :password");
    query.bindValue(":username", username);
    query.bindValue(":password", password);

    if (!query.exec()) {
        qDebug() << "Query Error: " << query.lastError().text();
        QMessageBox::warning(this, "Database Error", "Query failed: " + query.lastError().text());
        return;
    }

    if (query.next()) {  // ✅ Valid login
        QMessageBox::information(this, "Success", "Logged in successfully!");

        // 🔥 Update skipLogin based on checkbox state
        bool keepSignedIn = ui->signedIn->isChecked();
        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE admin SET skipLogin = :skip");
        updateQuery.bindValue(":skip", keepSignedIn ? 1 : 0);

        if (!updateQuery.exec()) {
            qDebug() << "Failed to update skipLogin: " << updateQuery.lastError().text();
        }

        // ✅ Open MainWindow
        MainWindow *mw = new MainWindow();
        mw->show();
        this->close();

    } else {  // ❌ Invalid login
        QMessageBox::warning(this, "Invalid", "Incorrect username or password");
    }
}
