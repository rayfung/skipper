#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "httpfileresource.h"
#include "tea.h"
#include "countermode.h"
#include <QCryptographicHash>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QtDebug>
#include <ctime>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->server = new HttpServer(this);
}

MainWindow::~MainWindow()
{
    delete this->server;
    delete ui;
}

void MainWindow::on_pushButtonStartServer_clicked()
{
    bool ok = this->server->start("127.0.0.1", 80, "D:/www/", new HttpFileResourceFactory());

    if(!ok)
    {
        QMessageBox::critical(this, QString("Skipper"), QString("Failed to start server!"));
    }
}
